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

  //Fallback vroot.h
  return DefaultRootWindow(display);
}

Display *XCowsayFactory::getOpenXServerDisplay() {
  return XOpenDisplay(getenv("DISPLAY"));
}

XCowsay XCowsayFactory::fromOptions(const Options &options) {
  Display *display = getOpenXServerDisplay();
  if (display == nullptr) {
    syslog(LOG_ERR, "Cannot open XServer display. $DISPLAY env is \"%s\"", getenv("DISPLAY"));
    exit(EXIT_FAILURE);
  }

  Window root = getRootWindow(display, options);
  if (root == 0) {
    syslog(LOG_ERR, "Cannot get root window. $XSCREENSAVER_WINDOW env is \"%s\"", getenv("XSCREENSAVER_WINDOW"));
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
