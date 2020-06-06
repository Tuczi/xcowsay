//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_OPTIONS_HPP
#define XCOWSAY_OPTIONS_HPP

#include <string>

namespace xcowsay {

struct Options {
  int delay;
  std::string cmd;
  std::string font;
  bool randomize = false;

  bool debug = false;
};

}
#endif //XCOWSAY_OPTIONS_HPP
