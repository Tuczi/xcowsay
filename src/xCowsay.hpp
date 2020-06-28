//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_XCOWSAY_HPP
#define XCOWSAY_XCOWSAY_HPP

#include <cstring>
#include <vector>
#include <X11/Xlib.h>
#include <unistd.h>

#include "ansi-esc-parser/csiParser.hpp"
#include "options.hpp"
#include "vroot.h"
#include "logger.hpp"

namespace xcowsay {

/**
 * Stores cursor position in Pixels
 */
class CursorPosition {
 private:
  XFontStruct *fontStruct;

  CursorPosition(uint startXPosition_, uint startYPosition, XFontStruct *fontStruct_)
      : fontStruct(fontStruct_), beginningOfNewline(startXPosition_), x(startXPosition_), y(startYPosition) {}

 public:
  uint beginningOfNewline;
  uint x;
  uint y;

  static CursorPosition fromOptions(const Options &options,
                                    const XWindowAttributes &windowAttributes,
                                    XFontStruct *fontStruct);
};

class XCowsay {
 private:
  static const size_t BUF_SIZE = 1024;

  Display *display;
  Window window;
  XWindowAttributes windowAttributes;
  GC gc;
  XFontStruct *fontStruct;
  Options options;
  CursorPosition cursorPosition;

  static bool tryReadLine(FILE *, std::string &);
  bool drawFrame();
  void clearDisplay(uint, uint);
  void clearLine(uint, uint);
  void deleteChar(uint, uint);
  void setCursorPosition(const ChangeCursorPosition &, uint);
  void displayText(const std::string_view &, uint);

 public:
  XCowsay(Display *display_,
          Window window_,
          XWindowAttributes windowAttributes_,
          GC gc_,
          XFontStruct *fontStruct_,
          Options options_)
      : display(display_),
        window(window_),
        windowAttributes(windowAttributes_),
        gc(gc_),
        fontStruct(fontStruct_),
        options(options_),
        cursorPosition(CursorPosition::fromOptions(options, windowAttributes, fontStruct)) {}

  void draw();
};

}
#endif //XCOWSAY_XCOWSAY_HPP
