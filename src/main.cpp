//
// Created by tuczi on 04.06.16.
//
#include "main.hpp"

Window get_root_window(Display* display) {
	char* end;
	const char* xscreensaver_window_env = getenv("XSCREENSAVER_WINDOW");

	if(xscreensaver_window_env != nullptr) {
		Window xscreensaver_window = (Window) std::strtoul(xscreensaver_window_env, &end, 0);

		if ((xscreensaver_window != 0)
			&& (end != nullptr)
			&& ((*end == ' ') || (*end == '\0'))
			&& (errno != ERANGE)) {

			return  xscreensaver_window;
		}
	}

	//Fallback to RootWindowOfScreen
	Window root_window = DefaultRootWindow(display);

	if(root_window != 0) {
		return root_window;
	}

	// TODO debug mode

	//Fallback to new windows - usefull for debug
	int screen = DefaultScreen(display);
	Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 24, 48, 860, 640, 1, WhitePixel(display, screen), BlackPixel(display, screen));
	XMapWindow(display, window);

	return window;
}

int main(int argc, char * argv[]) {
	const option_t options = get_options(argc, argv);

	/* open the display (connect to the X server) */
	Display* display = XOpenDisplay(getenv("DISPLAY"));
	if(display == nullptr) {
		//TODO error message
		exit(EXIT_FAILURE);
	}

	Window root = get_root_window(display);
	if(root == 0) {
		//TODO error message
		exit(EXIT_FAILURE);
	}

	/* get attributes of the root window */
	XWindowAttributes window_attributes;
	XGetWindowAttributes(display, root, &window_attributes);

	/* create a GC for drawing in the window */
	GC gc = XCreateGC(display, root, 0, nullptr);

	/* load a font */
	Font font = XLoadFont(display, options.font.c_str());
	//TODO error handling
	XSetFont(display, gc, font);

	/* get font metrics */
	XGCValues v;
	XGetGCValues(display, gc, GCFont, &v);
	XFontStruct* font_struct = XQueryFont(display, v.font);


	const int screen = DefaultScreen(display);
	XSetWindowBackground(display, root, BlackPixel(display, screen));

	while (true) {
		XClearWindow(display, root);

		draw(display, root, window_attributes, gc, font_struct, options);

		XFlush(display);
		sleep(options.delay);
	}

	XCloseDisplay(display);
}
