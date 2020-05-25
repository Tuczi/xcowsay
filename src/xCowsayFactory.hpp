//
// Created by tk on 15/03/2020.
//

#ifndef XCOWSAY_XCOWSAYFACTORY_HPP
#define XCOWSAY_XCOWSAYFACTORY_HPP

#include "xCowsay.hpp"
#include "logger.hpp"

namespace xcowsay {

class XCowsayFactory {
 private:
  static Window getRootWindow(Display *, const xcowsay::Options &);
  static Display *getOpenXServerDisplay();

 public:
  static xcowsay::XCowsay fromOptions(const xcowsay::Options &);
};

}

#endif //XCOWSAY_XCOWSAYFACTORY_HPP
