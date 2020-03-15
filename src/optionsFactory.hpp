//
// Created by tk on 15/03/2020.
//

#ifndef XCOWSAY_OPTIONSFACTORY_HPP
#define XCOWSAY_OPTIONSFACTORY_HPP

#include <getopt.h>
#include "options.hpp"

namespace xcowsay {

class OptionsFactory {
 private:
  static const struct option longopts[];

 public:
  static Options fromArgs(int argc, char *argv[]);
};

}
#include "options.hpp"

#endif //XCOWSAY_OPTIONSFACTORY_HPP
