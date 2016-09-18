//
// Created by tuczi on 04.06.16.
//

#include "main.hpp"

int main(int argc, char * argv[]) {
    const option_t options = get_options(argc, argv);

    /* open the display (connect to the X server) */
    Display *dpy = XOpenDisplay(getenv("DISPLAY"));

    std::vector<x_screen_attr_t> screens = get_screens_attr(dpy, options);
    /* draw something */
    while (1) {
        for(x_screen_attr_t& it: screens)
          XClearWindow(dpy, it.root);

        draw(dpy, screens, options);

        /* flush changes and sleep */
        XFlush(dpy);
        sleep(options.delay);
    }

    XCloseDisplay(dpy);
}
