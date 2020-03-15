//
// Created by tk on 15/03/2020.
//

#ifndef XCOWSAY_SRC_CSI_HPP_CSIPARSEREXCEPTION_HPP_
#define XCOWSAY_SRC_CSI_HPP_CSIPARSEREXCEPTION_HPP_

#include <exception>

class CsiParserException : public std::exception {
};

class IncorrectCsiSequenceException : CsiParserException {
  const char *what() const throw() {
    return "Incorrect CSI sequence";
  }
};

class PartialCsiSequenceException : CsiParserException {
  const char *what() const throw() {
    return "Partial CSI sequence";
  }
};

#endif //XCOWSAY_SRC_CSI_HPP_CSIPARSEREXCEPTION_HPP_
