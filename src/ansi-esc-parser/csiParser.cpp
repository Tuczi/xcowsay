//
// Created by tk on 15/03/2020.
//

#include "csiParser.hpp"
#include "csiParserException.hpp"

namespace xcowsay {
/**
 * returns
 *  valid CSI 24-bit color if success
 * 	otherwise throws CsiParserException
**/
uint32_t CsiParser::getExtendedColor() {
  //TODO what if previous character was not ';'?
  const auto modeCode = readCsiInt();
  //TODO validate if does not end with 'm'

  if (modeCode == 5) {
    buffer.remove_prefix(2);
    const auto colorCode = readCsiInt();

    return CSI_COLORS_MAP[colorCode];
  } else if (modeCode == 2) {
    buffer.remove_prefix(2);
    const auto r = readCsiInt();
    //TODO validate if does not end with 'm'

    buffer.remove_prefix(1);
    const auto g = readCsiInt();
    //TODO validate if does not end with 'm'

    buffer.remove_prefix(1);
    const auto b = readCsiInt();

    return (r << 16) | (g << 8) | b;
  }

  throw IncorrectCsiSequenceException();
}
/**
 * returns
 *  valid CSI code if success
 * 	otherwise throws CsiParserException
**/
uint32_t CsiParser::readCsiInt(int defaultValue) {
  int csiCodeCandidate = defaultValue;

  if(isCsiEndChar(buffer.front())) {
    return csiCodeCandidate;
  }

  auto[end, ec] = std::from_chars(buffer.data(), buffer.data() + buffer.size(), csiCodeCandidate);
  if (ec != std::errc()) {
    throw IncorrectCsiSequenceException();
  }

  // full code has been read
  if ((*end == CSI_SEPARATOR) || isCsiEndChar(*end)) {
    buffer.remove_prefix(end - buffer.data());
    return csiCodeCandidate;
  }

  if (csiCodeCandidate < 0 || csiCodeCandidate > 0xFF) {
    throw IncorrectCsiSequenceException();
  }

  if (*end == '\0') {
    throw PartialCsiSequenceException();
  }

  throw IncorrectCsiSequenceException();
}

bool CsiParser::isCsiEndChar(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

char CsiParser::getCsiType() {
  for (char c : buffer) {
    if (isCsiEndChar(c)) {
      return c;
    }
  }

  return '\0';
}
/**
 * Returns parsed CSI or partially parsed CSI code.
 * Internal buffer is maintained.
 *
 * Partial CSI code - it might happen that buffer ends in the middle of the CSI sequence. Scenarios:
 * 1. Method is not able to read code e.g. buffer contains only "3" instead of "35m" or "3m"
 * 2. Method is not able to read extended colors codes e.g. buffer contains "38;2;2" instead of "38;2;201;" or "38;2;202;"
 *
 * If partially parsed CSI is returned then buffer is rewind to the end of last fully read code position.
**/
CsiStringFragment CsiParser::parseCsiSequence() {
  char type = getCsiType();
  if (type == '\0') { // partial sequence - need to read more
    partialCsiSequenceBuffer = CSI_START_SEQUENCE;
    partialCsiSequenceBuffer += buffer;
    buffer = {};
    return currentFragment.withKeptColor();
  } else if (type == 'H') { // move cursor
    auto cursorPosition = parseCursorMove();
    return currentFragment.withAction(Action(cursorPosition));
  } else if (type == 'J') { // clear display
    auto clearDisplay = parseDisplayClear();
    return currentFragment.withAction(Action(clearDisplay));
  } else if (type == 'm') { // change graphic attribute
    auto graphicAttributes = parseGraphicAttributes();
    return CsiStringFragment(graphicAttributes);
  } else {
    //TODO unsupported CSI type
    return currentFragment.withKeptColor();
  }
}

//TODO
SetCursorPosition CsiParser::parseCursorMove() {
  uint column = readCsiInt(1);
  uint line = 1;

  if (buffer.front() == CSI_SEPARATOR) {
    buffer.remove_prefix(1);
    line = readCsiInt(1);
  }

  if (!isCsiEndChar(buffer.front())) { // TODO compare exact char
    buffer.remove_prefix(1);
    throw IncorrectCsiSequenceException();
  }

  if (column <= 0 || line <= 0) {
    throw IncorrectCsiSequenceException();
  }

  buffer.remove_prefix(1);
  return SetCursorPosition(column, line);
}

ClearDisplay CsiParser::parseDisplayClear() {
  uint mode = readCsiInt();

  if (mode > 3) {
    throw IncorrectCsiSequenceException();
  }

  if (!isCsiEndChar(buffer.front())) { // TODO compare exact char
    buffer.remove_prefix(1);
    throw IncorrectCsiSequenceException();
  }

  buffer.remove_prefix(1);
  return ClearDisplay(mode);
}

GraphicRendition CsiParser::parseGraphicAttributes() {
  GraphicRendition &csi = currentFragment.color;
  //TODO now partial sequence is impossible
  while (true) {
    std::string_view originalBuffer = buffer;
    try {
      csi = parseSGRSubsequence(csi);

      if (buffer.front() == '\0') {
        throw PartialCsiSequenceException();
      }

      if (isCsiEndChar(buffer.front())) {//TODO check if 'm'
        buffer.remove_prefix(1);
        return csi;
      }

      if (buffer.front() != CSI_SEPARATOR) {
        throw IncorrectCsiSequenceException();
      }

      buffer.remove_prefix(1);
    } catch (PartialCsiSequenceException &e) {
      partialCsiSequenceBuffer = CSI_START_SEQUENCE;
      partialCsiSequenceBuffer += originalBuffer;
      buffer = {};
      return csi;
    }
  }
}

GraphicRendition &CsiParser::parseSGRSubsequence(GraphicRendition &csi) {
  auto code = readCsiInt();
  if (code == 0) {
    csi = GraphicRendition();
  } else if (code == 1) {
    csi.bold = true;
  } else if (code >= 30 && code <= 37) {
    csi.fg_color = CSI_COLORS_MAP[code - 30];
  } else if (code == 38) {
    buffer.remove_prefix(3);
    //TODO validate if does not end with 'm'
    csi.fg_color = getExtendedColor();
  } else if (code == 39) {
    csi.fg_color = GraphicRendition().fg_color;
  } else if (code >= 40 && code <= 47) {
    csi.bg_color = CSI_COLORS_MAP[code - 40];
  } else if (code == 48) {
    buffer.remove_prefix(3);
    //TODO validate if does not end with 'm'
    csi.bg_color = getExtendedColor();
  } else if (code == 49) {
    csi.bg_color = GraphicRendition().bg_color;
  }

  return csi;
}

void CsiParser::parseNextFragment() {
  if (!partialCsiSequenceBuffer.empty()) {
    partialCsiSequenceBuffer += buffer;
    moveBuffer(std::move(partialCsiSequenceBuffer));
  }

  size_t escapePosition = buffer.find(ESC_CHAR);
  if (escapePosition == std::string::npos) { // buffer is simple string
    currentFragment = currentFragment.withBuffer(buffer);

    buffer = {};
    return;
  }

  // buffer ends with ESC character - rest of the string can be in the next buffer
  if (escapePosition >= buffer.length()) {
    //TODO check condition
    partialCsiSequenceBuffer = ESC_CHAR;
    currentFragment = currentFragment.withBuffer(buffer.substr(buffer.length() - 1));
    buffer = {};
    return;
  }

  if (buffer[escapePosition + 1] == CSI_OPEN_BRACKET) { // buffer has CSI start sequence
    if (escapePosition == 0) { // buffer starts with CSI start sequence
      buffer.remove_prefix(2);
      currentFragment = parseCsiSequence();

      return;
    }

    // there is some text before CSI start sequence
    currentFragment = currentFragment.withBuffer(buffer.substr(0, escapePosition));
    buffer.remove_prefix(escapePosition);

    return;
  }

  // buffer has just a ESC character without corresponding '['.
  // its unsupported ESC sequence. Display it as normal text.
  currentFragment = currentFragment.withBuffer(buffer.substr(escapePosition + 1));
  buffer.remove_prefix(escapePosition + 1);
}

bool CsiParser::hasNextFragment() {
  return !buffer.empty();
}

Action CsiParser::getCurrentAction() {
  return currentFragment.action;
}

void CsiParser::moveBuffer(std::string &&stringBuffer) {
  originalStringBuffer = std::move(stringBuffer);
  buffer = originalStringBuffer;
}
}
