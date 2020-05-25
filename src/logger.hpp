//
// Created by tk on 25/05/2020.
//

#ifndef XCOWSAY_SRC_LOGGER_HPP_
#define XCOWSAY_SRC_LOGGER_HPP_

#include <syslog.h>

namespace xcowsay {

class LoggerInitializer {
 public:
  LoggerInitializer() {
    openlog("xcowsay", LOG_PID | LOG_CONS, LOG_LOCAL0);
  }

  ~LoggerInitializer() {
    closelog();
  }
};

}

#endif //XCOWSAY_SRC_LOGGER_HPP_
