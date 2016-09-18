//
// Created by tuczi on 04.06.16.
//

#include "main.hpp"

int main(int argc, char * argv[]) {
    Display *dpy;
    Window root;
    XWindowAttributes wa;
    GC g;

    Font f;
    XFontStruct *fs;
    XGCValues v;

    const option_t options = get_options(argc, argv);

    /* open the display (connect to the X server) */
    dpy = XOpenDisplay(getenv("DISPLAY"));

    /* get the root window */
    //Screen* screen = DefaultScreenOfDisplay(dpy);
    root = DefaultRootWindow(dpy);//RootWindowOfScreen(screen);//

    /* get attributes of the root window */
    XGetWindowAttributes(dpy, root, &wa);

    /* create a GC for drawing in the window */
    g = XCreateGC(dpy, root, 0, nullptr);

    /* load a font */
    f = XLoadFont(dpy, options.font.c_str());
    XSetFont(dpy, g, f);

    /* get font metrics */
    XGetGCValues(dpy, g, GCFont, &v);
    fs = XQueryFont(dpy, v.font);

    /* draw something */
    while (1) {
        XClearWindow(dpy, root);

        draw(dpy, root, wa, g, fs, options);

        /* flush changes and sleep */
        XFlush(dpy);
        sleep(options.delay);
    }

    XCloseDisplay(dpy);
}
