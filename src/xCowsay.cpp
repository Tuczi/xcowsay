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
    //TODO need to reset it here so XClearWindow can use it. Not sure why window attribute is not stored. Maybe because of XFlush 
    XSetWindowBackground(display, window, BlackPixel(display, screen));

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

  const int lineHeight = fontStruct->ascent + fontStruct->descent;
  int positionX = random() % (windowAttributes.width / 2);
  int positionY = random() % (windowAttributes.height / 2) + 10;

  CsiParser parser;
  bool endOfPipe;
  do { //TODO refactor nested loops
    bool fullLineRead;
    int positionXTmp = positionX;
    do {
      std::string buffer(BUF_SIZE, '\0');//TODO use screen width as buffer size or config param
      endOfPipe = !tryReadLine(pipe, buffer);
      if (endOfPipe) {
        //TODO print the rest of buffer in ansi-esc-parser
        endOfPipe = true;
        break;
      }
      fullLineRead = (buffer.back() == '\n');
      if (fullLineRead) {
        buffer.resize(buffer.size() - 1);
      }

      parser.moveBuffer(std::move(buffer));
      while (parser.hasNextFragment()) {
        parser.parseNextFragment();
        auto stringFragment = parser.getCurrentStringFragment();

        XSetForeground(display, gc, stringFragment.color.fg_color);
        XDrawString(display,
                    window,
                    gc,
                    positionXTmp,
                    positionY,
                    stringFragment.str.data(),
                    stringFragment.str.size());

        positionXTmp += XTextWidth(fontStruct, stringFragment.str.data(), stringFragment.str.size());
      }
    } while (!fullLineRead);

    positionY += lineHeight;
  } while (!endOfPipe);

  pclose(pipe);
  return true;
}

}
