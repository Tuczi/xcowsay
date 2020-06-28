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
static const std::string CSI_START_SEQUENCE = "\x1B[";

/**
 * ANSI escape sequences
 *
 * There are multiple types of escape sequences. Sequences have different lengths and all starts with
 * ESC (0x1B) character followed by ASCII character from range: 0x40–0x5F.
 * This parser supports only control sequences - CSI (Control Sequence Introducer) which start with "ESC[".
 *
 * Control sequences control cursor moves (position), erasing (clearing) display, graphic rendition and more.
 * Syntax: CSI<int_sequence><final_character>
 * <int_sequence> is ';' separated integer sequence
 * <final_character> is single ASCII letter (A-Za-z)
 *
 * This parser supports only:
 * - CUP (Cursor position) sequences: CSI<n>;<m>H
 * - ED (Erase display) sequences: CSI<n>J
 * - SGR (Select Graphic Rendition) sequences: CSI<SGR_sequence>m
 * - TODO do we need full cursor manipulation? CSI<n>[A-H]
 *
 * CUP (Cursor position) sequences set cursor position.
 * Syntax: CSI<n>;<m>H
 * <n> is optional row
 * <m> is optional column
 * The values are 1-based.
 * Examples:
 * - CSI2;3H - set cursor to 2nd row and 3rd column
 * - CSI;3H - set cursor to 1st row (default) and 3rd column
 * - CSI2;H - set cursor to 2nd row and 1st column (default)
 * - CSIH - set cursor to 1st row (default) and 1st column (default)
 *
 * ED (Erase display) sequences clear display.
 * Syntax: CSI<n>J
 * <n> is optional and can have following values:
 * - 0 - (default)  clear from the cursor position to end of the screen
 * - 1 - clear from the cursor to beginning of the screen
 * - 2 - clear entire screen
 * - 3 - clear entire screen
 * Value 3 should also clear scrollback buffer which is not available in this parser.
 *
 * SGR sequences control graphic effects (attributes) like bold, italic, foreground color, background color and more.
 * Each effect (attribute) stays active until it is overridden or reset.
 * Multiple attributes can be set in one SGR sequence.
 * Syntax: CSI<SGR_sequence>m
 * <SGR_sequence> is ';' separated integer sequence which might be contextual.
 *
 * Integers are interpreted as:
 * - 0 - reset (default)
 * - 30–37 - set simple foreground color
 * - 38 - set foreground, next arguments are 5;<n> or 2;<r>;<g>;<b>, see examples
 * - 40–47 - set simple background color
 * - 48 - set background, next arguments are 5;<n> or 2;<r>;<g>;<b>, see examples
 *
 * Other values are currently unsupported.
 *
 * Setting foreground/background colors:
 * - set foreground color (8bit) from lookup table: CSI38;5;<n> m
 * - set background color (8bit) from lookup table: CSI48;5;<n> m
 * - set RGB (24bit) foreground color: CSI38;2;<r>;<g>;<b> m
 * - set RGB (24bit) background color: CSI48;2;<r>;<g>;<b> m
 *
 * Examples:
 * - CSIm - reset
 * - CSI0m - reset
 * - TODO more
 *
 * CsiParser is state full parser of CSI sequences.
 *
 * Typical usage:
 *
 * CsiParser parser;
 * parser.moveBuffer(std::move(buffer));
 * while (parser.hasNextFragment()) {
 *   parser.parseNextFragment();
 *   auto stringFragment = parser.getCurrentAction();
 * }
 */
class CsiParser {
 private:
  CsiStringFragment currentFragment;
  // holds original buffer value to satisfy string_view memory management
  std::string originalStringBuffer;
  std::string_view buffer;
  std::string partialCsiSequenceBuffer;

  inline bool isCsiEndChar(char c);
  char getCsiType();

  uint32_t getExtendedColor();
  uint32_t readCsiInt(int=0);
  uint parseSingleIntCsiSequence();
  CsiStringFragment parseCsiSequence();
  GraphicRendition& parseSGRSubsequence(GraphicRendition&);
  ChangeCursorPosition parseCursorMove();
  GraphicRendition parseGraphicAttributes();

 public:
  void moveBuffer(std::string&&);
  bool hasNextFragment();
  Action getCurrentAction();
  void parseNextFragment();
};
}

#endif //XCOWSAY_SRC_CSI_CPP_CSIPARSER_HPP_
