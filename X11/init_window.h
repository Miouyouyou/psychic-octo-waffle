#ifndef INIT_WINDOW_INCLUDED
#define INIT_WINDOW_INCLUDED 1

#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include "myy.h"

/* eglplatform.h implicitly include X11 libraries */
static Atom destroy;

struct _escontext
{
  /// Native System informations
  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;
  uint16_t window_width, window_height;
  /// EGL display
  EGLDisplay  display;
  /// EGL context
  EGLContext  context;
  /// EGL surface
  EGLSurface  surface;

};

void CreateNativeWindow(char* title, int width, int height);
EGLBoolean CreateEGLContext();
EGLBoolean CreateWindowWithEGLContext(char *title, int width, int height);
unsigned int UserInterrupt();
void RefreshWindow();

#endif
