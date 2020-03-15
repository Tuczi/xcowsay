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
uint32_t CsiParser::readCsiInt() {
  int csiCodeCandidate;
  auto[end, ec] = std::from_chars(buffer.data(), buffer.data() + buffer.size(), csiCodeCandidate);

  if (ec != std::errc()) {
    throw IncorrectCsiSequenceException();
  }

  // full code has been read
  if ((*end == CSI_SEPARATOR) || (*end == CSI_END)) {
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
Csi CsiParser::parseCsiSequence(Csi csi) {
  while (true) {
    std::string_view oryginalBuffer = buffer;
    try {
      csi = parseCsiSubsequence(csi);

      if (buffer.front() == '\0') {
        throw PartialCsiSequenceException();
      }

      if (buffer.front() == CSI_END) {
        buffer.remove_prefix(1);
        return csi;
      }

      if (buffer.front() != CSI_SEPARATOR) {
        throw IncorrectCsiSequenceException();
      }

      buffer.remove_prefix(1);
    } catch (PartialCsiSequenceException &e) {
      partialCsiSequenceBuffer = CSI_START_SEQUENCE;
      partialCsiSequenceBuffer += oryginalBuffer;
      buffer = {};
      return csi;
    }
  }
}
Csi &CsiParser::parseCsiSubsequence(Csi &csi) {
  auto code = readCsiInt();
  if (code == 0) {
    csi = Csi();
  } else if (code == 1) {
    csi.bold = true;
  } else if (code >= 30 && code <= 37) {
    csi.fg_color = CSI_COLORS_MAP[code - 30];
  } else if (code == 38) {
    buffer.remove_prefix(3);
    //TODO validate if does not end with 'm'
    csi.fg_color = getExtendedColor();
  } else if (code == 39) {
    csi.fg_color = Csi().fg_color;
  } else if (code >= 40 && code <= 47) {
    csi.bg_color = CSI_COLORS_MAP[code - 40];
  } else if (code == 48) {
    buffer.remove_prefix(3);
    //TODO validate if does not end with 'm'
    csi.bg_color = getExtendedColor();
  } else if (code == 49) {
    csi.bg_color = Csi().bg_color;
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
    currentFragment = CsiStringFragment(currentFragment.color, buffer);

    buffer = {};
    return;
  }

  // buffer ends with ESC character - rest of the string can be in the next buffer
  if (escapePosition >= buffer.length()) {
    //TODO check condition
    partialCsiSequenceBuffer = ESC_CHAR;
    currentFragment = CsiStringFragment(currentFragment.color, buffer.substr(buffer.length() - 1));
    buffer = {};
    return;
  }

  if (buffer[escapePosition + 1] == CSI_OPEN_BRACKET) { // buffer has CSI start sequence
    if (escapePosition == 0) { // buffer starts with CSI start sequence
      buffer.remove_prefix(2);
      auto color = parseCsiSequence(currentFragment.color);
      currentFragment = CsiStringFragment(color, {});

      return;
    }

    // there is some text before CSI start sequence
    currentFragment = CsiStringFragment(currentFragment.color, buffer.substr(0, escapePosition));
    buffer.remove_prefix(escapePosition);

    return;
  }

  // buffer has just a ESC character without corresponding '['
  currentFragment = CsiStringFragment(currentFragment.color, buffer.substr(escapePosition + 1));
  buffer.remove_prefix(escapePosition + 1);
}
/**
* returns:
*   true if has next fragment to pare (buffer has not been fully read)
*   false otherwise
**/
bool CsiParser::hasNextFragment() {
  return !buffer.empty();
}
CsiStringFragment CsiParser::getCurrentStringFragment() {
  return currentFragment;
}
void CsiParser::moveBuffer(std::string &&stringBuffer) {
  originalStringBuffer = std::move(stringBuffer);
  buffer = originalStringBuffer;
}
}
