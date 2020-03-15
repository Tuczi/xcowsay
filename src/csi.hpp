//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_CSI_HPP
#define XCOWSAY_CSI_HPP

#include "csiColors.hpp"
#include <exception>
#include <string>

namespace xcowsay {
//TODO implement bold
struct Csi {
  bool bold;
  uint32_t fg_color;
  uint32_t bg_color;

  Csi() : bold(false), fg_color(0xFFFFFF), bg_color(0) {}

  Csi(uint32_t fg_color_, uint32_t bg_color_) : bold(false), fg_color(fg_color_), bg_color(bg_color_) {}
};

struct CsiStringFragment {
  Csi color;
  std::string_view str;

  CsiStringFragment(Csi color_, std::string_view str_) : color(color_), str(str_) {}
};

}
#endif //XCOWSAY_CSI_HPP
