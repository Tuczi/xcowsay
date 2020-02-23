//
// Created by tuczi on 04.06.16.
//

#include "xcowsay.hpp"

namespace xcowsay {

/**
 * returns:
 *   true if pipe is not end of pipe
 *   false otherwise
 **/
bool XCowsay::readLine(FILE* file, std::string& buffer) {
	char* line = fgets((char*) buffer.c_str(), buffer.length(), file);
	if (line == nullptr)
		return false;

	size_t len = strnlen(buffer.c_str(), buffer.length());
	buffer.resize(len - 1);

	return true;
}

void XCowsay::draw() {
	while (true) {
		XClearWindow(display, window);

		//TODO handle partially displayed frames
		drawFrame();

		XFlush(display);
		sleep(options.delay);
	}

	XCloseDisplay(display);
}
/**
 * returns
 *   true if fram was fully rendered
 *   false otherwise
 **/
bool XCowsay::drawFrame() {
	FILE* pipe = popen(options.cmd.c_str(), "r");
	if (pipe == nullptr) {
		return false;
	}

	const int lineHeight = fontStruct != nullptr ? fontStruct->ascent + fontStruct->descent : 13;//TODO method getLineHight
	int positionX = random() % (windowAttributes.width / 2);
	int positionY = random() % (windowAttributes.height / 2) + 10; //TODO method getRandomWindowPosition

	CsiParser parser;
	while (true) {
		std::string buffer(BUF_SIZE, '\0');
		const bool endOfPipe = !readLine(pipe, buffer);
		if(endOfPipe) {
			//TODO print the rest of buffer in parser
			break;
		}

		int positionXTmp = positionX;
		parser.moveBuffer(std::move(buffer));
		while(parser.hasNextFragment()) {
			parser.parseNextFragment();
			auto string_fragment = parser.getCurrentStringFragment();

			XSetForeground(display, gc, string_fragment.color.fg_color);
			XDrawString(display, window, gc, positionXTmp, positionY, string_fragment.str, string_fragment.len);
			positionXTmp += XTextWidth(fontStruct, string_fragment.str, string_fragment.len);
		}
		positionY += lineHeight;
	}

	pclose(pipe);
	return true;
}

Window XCowsayFactory::getRootWindow(Display* display, Options options) {
	if(options.debug) {
		//Use new standalone window
		int screen = DefaultScreen(display);
		Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 24, 48, 860, 640, 1, WhitePixel(display, screen), BlackPixel(display, screen));
		XMapWindow(display, window);

		return window;
	}

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

	//Fallback vroot.h
	return DefaultRootWindow(display);
}

Display* XCowsayFactory::getOpenXServerDisplay() {
	return XOpenDisplay(getenv("DISPLAY"));
}

XCowsay XCowsayFactory::fromOptions(Options options) {
	Display* display = getOpenXServerDisplay();
	if(display == nullptr) {
		//TODO error message
		exit(EXIT_FAILURE);
	}

	Window root = getRootWindow(display, options);
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
	//TODO error handling

	int screen = DefaultScreen(display);
	XSetWindowBackground(display, root, BlackPixel(display, screen));

	return XCowsay(display, root, window_attributes, gc, font_struct, options);
}

}
