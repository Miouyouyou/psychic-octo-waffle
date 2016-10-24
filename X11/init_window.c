#include "init_window.h"

#include <string.h>

struct _escontext ESContext = {
  .native_display = NULL,
  .window_width = 0,
  .window_height = 0,
  .native_window  = 0,
  .display = NULL,
  .context = NULL,
  .surface = NULL
};

#define TRUE 1
#define FALSE 0

void CreateNativeWindow(char* title, int width, int height)
{
  Window root;
  XSetWindowAttributes swa;
  XSetWindowAttributes xattr;
  Atom wm_state;
  XWMHints hints;
  XEvent xev;
  Window win;
  Atom wm_delete;
  Display* x_display = XOpenDisplay(NULL);

  root = DefaultRootWindow(x_display);

  swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask;

  /* XCreateWindow arguments :
     Display,
     Parent window,
     x, y from Parent Window
     width, height of the new window,
     Border width,
     Depth,
     Window class,
     VisualType,
     MasksType,
     Masks */

  win = XCreateWindow(x_display, root, 0, 0, width, height,
                      0, CopyFromParent, InputOutput, CopyFromParent,
                      CWEventMask, &swa);

  xattr.override_redirect = FALSE;

  XChangeWindowAttributes( x_display, win, CWOverrideRedirect, &xattr );

  /* Tells the window manager to accept input for this new window */
  hints.input = TRUE;
  hints.flags = InputHint;

  XSetWMHints(x_display, win, &hints);

  /* Without this, the window would not be visible
     Quoting XChangeWindowAttributes manual :
   The created window is not yet displayed (mapped) on the user's
   display. To display the window, call XMapWindow. */

  XMapWindow(x_display, win);

  /* Sets the title. Can it be done before Mapping the window ? */
  XStoreName(x_display, win, title);

  /* Get the ATOM "_NET_WM_STATE",
     Create it if it doesn't exist

   The XInternAtom function returns the atom identifier associated with
   the specified atom_name string.  If only_if_exists is False, the atom
   is created if it does not exist. */
  wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage; /* XClientMessageEvent */
  xev.xclient.window       = win;
  xev.xclient.message_type = wm_state;
  xev.xclient.format       = 32;
  /* _NET_WM_STATE_REMOVE        0    remove/unset property
     _NET_WM_STATE_ADD           1    add/set property
     _NET_WM_STATE_TOGGLE        2    toggle property  */
  xev.xclient.data.l[0]    = 1;
  xev.xclient.data.l[1]    = FALSE;
  XSendEvent (
    x_display,
    root, // The example states DefaultRootWindow( x_display ) ??
    FALSE,
    SubstructureNotifyMask,
    &xev );

  /* So, this seems to be needed in order to catch the destruction of
     the window, when scanning for events
     The lines topped with a 'fatal IO error 11 BUG' concern this
     problem */

  destroy = XInternAtom(x_display, "WM_DELETE_WINDOW", FALSE);
  XSetWMProtocols(x_display, win, &destroy, 1);

  ESContext.native_display = x_display;
  ESContext.window_width = width;
  ESContext.window_height = height;
  ESContext.native_window = (EGLNativeWindowType) win;
}

EGLBoolean CreateEGLContext ()
{
   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;
   EGLContext context;
   EGLSurface surface;
   EGLConfig config;
   EGLint fbAttribs[] =
   {
       EGL_RED_SIZE,        5,
       EGL_GREEN_SIZE,      6,
       EGL_BLUE_SIZE,       5,
       EGL_ALPHA_SIZE,      8,
       EGL_DEPTH_SIZE,     16,
       EGL_NONE
   };
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
   EGLDisplay display = eglGetDisplay( ESContext.native_display );
   if ( display == EGL_NO_DISPLAY )
   {
      return EGL_FALSE;
   }

   // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
   {
      return EGL_FALSE;
   }

   // Get configs
   if ( (eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0))
   {
      return EGL_FALSE;
   }

   // Choose config
   if ( (eglChooseConfig(display, fbAttribs, &config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1))
   {
      return EGL_FALSE;
   }

   // Create a surface
   surface = eglCreateWindowSurface(display, config, ESContext.native_window, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return EGL_FALSE;
   }

   // Create a GL context
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return EGL_FALSE;
   }

   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
   {
      return EGL_FALSE;
   }

   ESContext.display = display;
   ESContext.surface = surface;
   ESContext.context = context;
   return EGL_TRUE;
}

EGLBoolean CreateWindowWithEGLContext(char *title, int width, int height) {
  CreateNativeWindow(title, width, height);
  return CreateEGLContext();
}

void RefreshWindow() { eglSwapBuffers(ESContext.display, ESContext.surface); }
unsigned int UserInterrupt() {

  XEvent xev;
  Display *x_display = ESContext.native_display;
  unsigned int interrupted = 0;

  while ( XPending( x_display ) ) {
    XNextEvent( x_display, &xev );
    switch(xev.type) {
      case ClientMessage:
        interrupted = (xev.xclient.data.l[0] == destroy);
        break;
      case ButtonPress:
        myy_click(xev.xbutton.x, ESContext.window_height - xev.xbutton.y, xev.xbutton.button);
        break;
      case MotionNotify:
        myy_move(xev.xmotion.x, ESContext.window_height - xev.xmotion.y);
        break;
      case KeyPress:
        myy_key(xev.xkey.keycode);
        break;
    }
  }

  return interrupted;
}

static void Terminate() {
  Display* display = ESContext.native_display;
  Window window = ESContext.native_window;

  if (window) XDestroyWindow( ESContext.native_display, window );

  XCloseDisplay( display );
}
