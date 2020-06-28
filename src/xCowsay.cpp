//
// Created by tuczi on 04.06.16.
//

#include "xCowsay.hpp"

namespace xcowsay {

/**
 * Read line from FILE*.
 * Stores newline chareacter if entire line has been read.
 * 
 * returns:
 *   true if pipe is not end of pipe
 *   false otherwise
 **/
bool XCowsay::tryReadLine(FILE *file, std::string &buffer) {
  char *line = fgets((char *) buffer.c_str(), buffer.length(), file);
  if (line == nullptr)
    return false;

  size_t len = strnlen(buffer.c_str(), buffer.length());
  buffer.resize(len);

  return true;
}

void XCowsay::draw() {
  while (true) {
    int screen = DefaultScreen(display);
    XSetWindowBackground(display, window, BlackPixel(display, screen));
    XSetForeground(display, gc, WhitePixel(display, screen));

    XClearWindow(display, window);

    //TODO handle partially displayed frames
    drawFrame();

    XFlush(display); //TODO XFlush or something else?
    //TODO what to do with xlib events? do the program need to handle them somehow? discard them?

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
  //TODO think if double buffering is needed
  FILE *pipe = popen(options.cmd.c_str(), "r");
  if (pipe == nullptr) {
    syslog(LOG_ERR, "Cannot open pipe. Pipe is \"%s\"", options.cmd.c_str());
    return false;
  }

  cursorPosition = CursorPosition::fromOptions(options, windowAttributes, fontStruct);
  const uint lineHeight = fontStruct->ascent + fontStruct->descent;

  CsiParser parser;
  bool endOfPipe;
  do { //TODO refactor nested loops
    std::string buffer(BUF_SIZE, '\0');//TODO use screen width as buffer size or config param
    endOfPipe = !tryReadLine(pipe, buffer);

    if (endOfPipe) {
      //TODO print the rest of buffer in ansi-esc-parser
      break;
    }

    const bool fullLineRead = (buffer.back() == '\n');
    if (fullLineRead) {
      buffer.resize(buffer.size() - 1);
    }

    parser.moveBuffer(std::move(buffer));
    while (parser.hasNextFragment()) {
      parser.parseNextFragment();
      const auto action = parser.getCurrentAction();

      switch(action.type) {
        case UPDATE_GRAPHIC_ATTRIBUTES:
          //TODO add stringFragment.color.bg_color support
          //TODO add stringFragment.color.bold support
          XSetForeground(display, gc, action.color.fg_color);
          //XSetBackground(display, gc, action.color.bg_color);
          break;

        case DISPLAY_TEXT:
          displayText(action.displayText, lineHeight);
          break;

        case CLEAR_DISPLAY:
          clearDisplay(action.singleIntCsi, lineHeight);
          break;

        case CLEAR_LINE:
          clearLine(action.singleIntCsi, lineHeight);
          break;

        case DELETE_CHAR:
          deleteChar(action.singleIntCsi, lineHeight);

        case SET_CURSOR_POSITION:
          setCursorPosition(action.cursorPosition, lineHeight);
          break;

        default:
          syslog(LOG_ERR, "Unhandled action of type: %d", action.type);
      }
    }
    if (fullLineRead) {
      cursorPosition.y += lineHeight;
      cursorPosition.x = cursorPosition.beginningOfNewline;
    }
  } while (!endOfPipe);

  pclose(pipe);
  return true;
}

void XCowsay::displayText(const std::string_view& str, const uint lineHeight) {
  const int stringDisplayWidth = XTextWidth(fontStruct, str.data(), str.size());
  XClearArea(display,
             window,
             cursorPosition.x,
             cursorPosition.y - fontStruct->ascent,
             stringDisplayWidth,
             lineHeight,
             false);
  XDrawString(display, window, gc, cursorPosition.x, cursorPosition.y, str.data(), str.size());
  XFlush(display);
  cursorPosition.x += stringDisplayWidth;
}

void XCowsay::setCursorPosition(const ChangeCursorPosition &setCursorPosition, const uint lineHeight) {
  if(setCursorPosition.column) {
    cursorPosition.x = (setCursorPosition.column - 1) * XTextWidth(fontStruct, " ", 1);
  }
  if(setCursorPosition.line) {
    cursorPosition.y = (setCursorPosition.line - 1) * lineHeight + fontStruct->ascent;
  }
}

void XCowsay::clearDisplay(const uint mode, const uint lineHeight) {
  switch (mode) {
    case 0: //clear from cursor to end of screen.
      //clear till the end of line
      XClearArea(display,
                 window,
                 cursorPosition.x,
                 cursorPosition.y - fontStruct->ascent,
                 -1,
                 lineHeight,
                 false);
      //clear rest of the screen
      XClearArea(display,
                 window,
                 0,
                 cursorPosition.y - fontStruct->ascent + lineHeight,
                 -1,
                 -1,
                 false);
      break;
    case 1: //clear from cursor to beginning of the screen
      //clear till the beginning of line
      XClearArea(display,
                 window,
                 0,
                 cursorPosition.y - fontStruct->ascent,
                 cursorPosition.x,
                 lineHeight,
                 false);
      //clear rest of the screen
      XClearArea(display,
                 window,
                 0,
                 0,
                 -1,
                 cursorPosition.y - fontStruct->ascent,
                 false);
      break;
    default: //clear entire display (mode 2 and 3)
      XClearWindow(display, window);
  }
}

void XCowsay::clearLine(const uint mode, const uint lineHeight) {
  //TODO test
  switch (mode) {
    case 0: //clear till the end of line
      XClearArea(display,
                 window,
                 cursorPosition.x,
                 cursorPosition.y - fontStruct->ascent,
                 -1,
                 lineHeight,
                 false);
      break;
    case 1: //clear till the beginning of line
      XClearArea(display,
                 window,
                 0,
                 cursorPosition.y - fontStruct->ascent,
                 cursorPosition.x,
                 lineHeight,
                 false);
      break;
    default: //clear entire line (mode 2 and 3)
      XClearArea(display,
                 window,
                 0,
                 cursorPosition.y - fontStruct->ascent,
                 -1,
                 lineHeight,
                 false);
  }
}

void XCowsay::deleteChar(const uint count, const uint lineHeight) {
  //TODO test and check if this is sufficient
  //copy line from current position + n characters into current position
  XCopyArea(display,
            window,
            window,
            gc,
            cursorPosition.x + count * XTextWidth(fontStruct, " ", 1),
            cursorPosition.y - fontStruct->ascent,
            -1,
            lineHeight,
            cursorPosition.x,
            cursorPosition.y - fontStruct->ascent);
}

CursorPosition CursorPosition::fromOptions(const Options &options,
                                           const XWindowAttributes &windowAttributes,
                                           XFontStruct *fontStruct) {
  uint positionX = options.randomize
                   ? random() % (windowAttributes.width / 2)
                   : 0;
  uint positionY = options.randomize
                   ? random() % (windowAttributes.height / 2) + fontStruct->ascent
                   : fontStruct->ascent;

  return CursorPosition(positionX, positionY, fontStruct);
}
}
