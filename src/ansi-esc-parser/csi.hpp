//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_CSI_HPP
#define XCOWSAY_CSI_HPP

#include "csiColors.hpp"
#include <exception>
#include <string>

namespace xcowsay {

struct GraphicRendition {
  bool bold; //TODO implement bold
  uint32_t fg_color;
  uint32_t bg_color;

  GraphicRendition() : bold(false), fg_color(0xFFFFFF), bg_color(0) {}

  GraphicRendition(uint32_t fg_color_, uint32_t bg_color_) : bold(false), fg_color(fg_color_), bg_color(bg_color_) {}
};

struct SetCursorPosition {
  uint column;
  uint line;

  SetCursorPosition(int column_, int line_) : column(column_), line(line_) {}
};

struct ClearDisplay {
  uint mode;

  ClearDisplay(int mode_) : mode(mode_) {}
};

enum ActionType { DISPLAY_TEXT, UPDATE_GRAPHIC_ATTRIBUTES, CLEAR_DISPLAY, SET_CURSOR_POSITION };

struct Action {
  ActionType type;
  union {
    std::string_view displayText;
    GraphicRendition color;
    SetCursorPosition setCursorPosition;
    ClearDisplay clearDisplay;
  };

  Action(std::string_view displayText_) : type(DISPLAY_TEXT), displayText(displayText_) {}
  Action(GraphicRendition color_) : type(UPDATE_GRAPHIC_ATTRIBUTES), color(color_) {}
  Action(SetCursorPosition setCursorPosition_) : type(SET_CURSOR_POSITION), setCursorPosition(setCursorPosition_) {}
  Action(ClearDisplay clearDisplay_) : type(CLEAR_DISPLAY), clearDisplay(clearDisplay_) {}
};

class CsiStringFragment {
 private:
  CsiStringFragment(GraphicRendition color_, Action action_) : color(color_), action(action_) {}
  GraphicRendition color;
  Action action;

  CsiStringFragment() : color(), action(Action("")) {}
  CsiStringFragment(GraphicRendition color_) : color(color_), action(Action(color_)) {}

  inline CsiStringFragment withAction(Action action_) {
    return CsiStringFragment(color, action_);
  }

  inline CsiStringFragment withBuffer(std::string_view str) {
    return CsiStringFragment(color, Action(str));
  }

  inline CsiStringFragment withKeptColor() {
    return CsiStringFragment(color, Action(""));
  }

  friend class CsiParser;
};

}
#endif //XCOWSAY_CSI_HPP
