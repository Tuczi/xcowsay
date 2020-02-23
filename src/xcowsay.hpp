//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_XCOWSAY_HPP
#define XCOWSAY_XCOWSAY_HPP

#include <cstring>
#include <vector>
#include <X11/Xlib.h>
#include <unistd.h>

#include "csi.hpp"
#include "args_parser.hpp"
#include "vroot.h"

namespace xcowsay {

class XCowsay {
	private:
		static const size_t BUF_SIZE = 1024;

		Display* display;
		Window window;
		XWindowAttributes windowAttributes;
		GC gc;
		XFontStruct* fontStruct;
		Options options;

		bool readLine(FILE*, std::string&);
		bool drawFrame();

	public:
		XCowsay(Display* display_, Window window_, XWindowAttributes windowAttributes_, GC gc_, XFontStruct* fontStruct_, Options options_)
			: display(display_),window(window_),windowAttributes(windowAttributes_), gc(gc_), fontStruct(fontStruct_), options(options_) { }

		void draw();
};

class XCowsayFactory {
	private:
		static Window getRootWindow(Display*, Options);
		static Display* getOpenXServerDisplay();

	public:
		static XCowsay fromOptions(Options);
};

}
#endif //XCOWSAY_XCOWSAY_HPP
