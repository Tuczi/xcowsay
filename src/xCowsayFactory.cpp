//
// Created by tk on 15/03/2020.
//

#include "xCowsayFactory.hpp"

namespace xcowsay {

Window XCowsayFactory::getRootWindow(Display *display, const Options &options) {
  if (options.debug) {
    //Use new standalone window
    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 24, 48, 860, 640, 1,
                                        WhitePixel(display, screen), BlackPixel(display, screen));
    XMapWindow(display, window);

    return window;
  }

  char *end;
  const char *xscreensaver_window_env = getenv("XSCREENSAVER_WINDOW");

  if (xscreensaver_window_env != nullptr) {
    auto xscreensaver_window = (Window) strtoul(xscreensaver_window_env, &end, 0);

    if ((xscreensaver_window != 0)
        && (end != nullptr)
        && ((*end == ' ') || (*end == '\0'))
        && (errno != ERANGE)) {

      return xscreensaver_window;
    }
  }

  syslog(LOG_INFO, "Cannot get root window from env. $XSCREENSAVER_WINDOW env is \"%s\". Using fallback", getenv("XSCREENSAVER_WINDOW"));
  //Fallback vroot.h
  return DefaultRootWindow(display);
}

Display *XCowsayFactory::getOpenXServerDisplay() {
  return XOpenDisplay(getenv("DISPLAY"));
}

int xErrorHandler(Display* display, XErrorEvent* e) {
  char msg[50];
  XGetErrorText(display, e->error_code, msg, sizeof(msg));

  syslog(LOG_ERR, "Xlib error %d: \"%s\". Request code %d. Minor code %d",
         e->error_code, msg, e->request_code, e->minor_code);

  return 0;
}

int xIoErrorHandler(Display*) {
  syslog(LOG_ERR, "Xlib io error");

  return 0;
}

XCowsay XCowsayFactory::fromOptions(const Options &options) {
  XSetErrorHandler(xErrorHandler);
  XSetIOErrorHandler(xIoErrorHandler);

  Display *display = getOpenXServerDisplay();
  if (display == nullptr) {
    syslog(LOG_ERR, "Cannot open XServer display. $DISPLAY env is \"%s\"", getenv("DISPLAY"));
    exit(EXIT_FAILURE);
  }

  Window root = getRootWindow(display, options);
  if (root == 0) {
    syslog(LOG_ERR, "Cannot get root window.");
    exit(EXIT_FAILURE);
  }

  /* get attributes of the root window */
  XWindowAttributes windowAttributes;
  XGetWindowAttributes(display, root, &windowAttributes);

  /* create a GC for drawing in the window */
  GC gc = XCreateGC(display, root, 0, nullptr);

  /* load a font */
  Font font = XLoadFont(display, options.font.c_str());
  //TODO error handling
  XSetFont(display, gc, font);

  /* get font metrics */
  XGCValues v;
  XGetGCValues(display, gc, GCFont, &v);
  XFontStruct *fontStruct = XQueryFont(display, v.font);
  if (fontStruct == nullptr) {
    syslog(LOG_ERR, "Cannot query font. Font is \"%s\"", options.font.c_str());
    exit(EXIT_FAILURE);
  }

  int screen = DefaultScreen(display);
  XSetWindowBackground(display, root, BlackPixel(display, screen));

  return xcowsay::XCowsay(display, root, windowAttributes, gc, fontStruct, options);
}
}
