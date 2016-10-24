#include <jni.h>
#include <errno.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#include <EGL/egl.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h> /* chdir */

/* Bad mkdir */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

extern void myy_init_display(NativeWindowType window);
extern void myy_init();
extern void myy_draw();
extern void myy_save();
extern void myy_stop();

extern int myy_animating();
extern int myy_pause();
extern int myy_resume();

extern void myy_click(int x, int y);
extern void myy_display_initialised(int, int);

extern uint8_t *scratch;

static struct egl_elements {
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
} egl;

static const EGLint attribs[] = {
  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
  EGL_NONE
};

static const EGLint GiveMeGLES2[] = {
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE
};

/**
 * Initialize an EGL context for the current display.
 */
static int add_egl_context_to(NativeWindowType window, struct egl_elements *e) {
  // initialize OpenGL ES and EGL

  /*
    * Here specify the attributes of the desired configuration.
    * Below, we select an EGLConfig with at least 8 bits per color
    * component compatible with on-screen windows
    */
  EGLint w, h, dummy, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);

  /* Here, the application chooses the configuration it desires. In this
    * sample, we have a very simplified selection process, where we pick
    * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
    * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
    * As soon as we picked a EGLConfig, we can safely reconfigure the
    * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  /* Implicitly provided by EGL on Android */
  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, window, NULL);
  context = eglCreateContext(display, config, NULL, GiveMeGLES2);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      return -1;
  }

  e->context = context;
  e->display = display;
  e->surface = surface;

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  myy_display_initialised(w, h);

  return 0;
}

static void egl_sync(struct egl_elements* e) {
  eglSwapBuffers(e->display, e->surface);
}

static void egl_stop(struct egl_elements* e) {
  if (e->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(e->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (e->context != EGL_NO_CONTEXT)
        eglDestroyContext(e->display, e->context);
    if (e->surface != EGL_NO_SURFACE)
        eglDestroySurface(e->display, e->surface);
    eglTerminate(e->display);
  }
  e->display = EGL_NO_DISPLAY;
  e->context = EGL_NO_CONTEXT;
  e->surface = EGL_NO_SURFACE;
}


/**
 * Shared state for our app.
 */

int animating;

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
  int pc = AMotionEvent_getPointerCount(event);
  for (int p = 0; p < pc; p++)
    myy_click(AMotionEvent_getX(event, p), AMotionEvent_getY(event, p));
  return 1;
}

static void goto_data_dir(const char* data_dir) {
  chdir(data_dir);
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {

  struct egl_elements *e = &egl;
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    myy_save();
    break;
  case APP_CMD_INIT_WINDOW:
    if (app->window != NULL) {
      add_egl_context_to(app->window, e);
      /* Terrible idea, and yet still better than using AAssetManager */
      mkdir("/sdcard/OpenGL", 0777);
      chdir("/sdcard/OpenGL");
      myy_init();
      myy_draw();
      egl_sync(e);
    }
    break;
  case APP_CMD_TERM_WINDOW:
    myy_stop();
    egl_stop(e);
    animating = 0;
    break;
  case APP_CMD_GAINED_FOCUS:
    animating = 1;
    egl_sync(e);
    break;
  case APP_CMD_LOST_FOCUS:
    animating = 0;
    break;
  }

}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* app) {

  // Make sure glue isn't stripped.
  app_dummy();

  app->onAppCmd = engine_handle_cmd;
  app->onInputEvent = engine_handle_input;

  // Copy assets files into the internalDataPath folder
  goto_data_dir(app->activity->internalDataPath);
  /*copy_assets_to_internal_memory(app->activity->assetManager);*/

  // loop waiting for stuff to do.

  struct egl_elements* e = &egl;
  while (1) {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident=ALooper_pollAll(animating ? 0 : -1, NULL, &events,
                                  (void**)&source)) >= 0) {

      // Process this event.
      if (source != NULL) { source->process(app, source); }

      // If a sensor has data, process it now.
      if (ident == LOOPER_ID_USER) {}

      // Check if we are exiting.
      if (app->destroyRequested != 0) {
        myy_stop();
        animating = 0;
        return;
      }
    }

    if (animating && app->window != NULL) {
      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      myy_draw();
      egl_sync(e);
    }
  }
}
