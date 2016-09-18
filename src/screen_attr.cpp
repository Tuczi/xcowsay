#include "screen_attr.hpp"

std::vector<x_screen_attr_t> get_screens_attr(Display *dpy, option_t options) {
    /* load a font */
    Font f = XLoadFont(dpy, options.font.c_str());

    const int screen_count = XScreenCount(dpy);

    std::vector<x_screen_attr_t> result(0);
    result.reserve(screen_count);
    for(int i=0; i<screen_count; i++) {
        Screen* screen = XScreenOfDisplay(dpy, i);
        x_screen_attr_t x_screen_attr;
        XGCValues v;

        /* get the root window */
        x_screen_attr.root = RootWindowOfScreen(screen);

        /* get attributes of the root window */
        XGetWindowAttributes(dpy, x_screen_attr.root, &x_screen_attr.wa);

        /* create a GC for drawing in the window */
        x_screen_attr.g = XCreateGC(dpy, x_screen_attr.root, 0, nullptr);

        XSetFont(dpy, x_screen_attr.g, f);
        /* get font metrics */
        XGetGCValues(dpy, x_screen_attr.g, GCFont, &v);
        x_screen_attr.fs = XQueryFont(dpy, v.font);

        result.push_back(x_screen_attr);
    }

    return result;
}
