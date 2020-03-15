//
// Created by tk on 15/03/2020.
//

#ifndef XCOWSAY_SRC_CSI_CPP_CSIPARSER_HPP_
#define XCOWSAY_SRC_CSI_CPP_CSIPARSER_HPP_

#include <charconv>
#include "csi.hpp"

namespace xcowsay {
static const char ESC_CHAR = '\x1B';
static const char CSI_OPEN_BRACKET = '[';
static const char CSI_SEPARATOR = ';';
static const char CSI_END = 'm';
static const std::string CSI_START_SEQUENCE = "\x1B[";

class CsiParser {
 private:
  CsiStringFragment currentFragment = xcowsay::CsiStringFragment(xcowsay::Csi(), {});
  std::string originalStringBuffer;
  std::string_view buffer;
  std::string partialCsiSequenceBuffer;

  uint32_t getExtendedColor();
  xcowsay::Csi parseCsiSequence(xcowsay::Csi);
  uint32_t readCsiInt();

 public:
  void moveBuffer(std::string &&);
  bool hasNextFragment();
  void parseNextFragment();
  xcowsay::CsiStringFragment getCurrentStringFragment();
  xcowsay::Csi &parseCsiSubsequence(xcowsay::Csi &csi);
};
}

#endif //XCOWSAY_SRC_CSI_CPP_CSIPARSER_HPP_
