//
// Created by tk on 15/03/2020.
//

#include "optionsFactory.hpp"

namespace xcowsay {
const struct option OptionsFactory::longopts[] = {
    {"cmd", required_argument, nullptr, 'c'},
    {"delay", required_argument, nullptr, 'd'},
    {"font", required_argument, nullptr, 'f'},
    {"randomize", no_argument, nullptr, 'r'},
    {"debug", no_argument, nullptr, 'D'},
    {nullptr, no_argument, nullptr, '\0'},
};

Options OptionsFactory::fromArgs(int argc, char *argv[]) {
  xcowsay::Options options;
  int index;
  int iarg = 0;

//turn off getopt error message
  opterr = 1;

  while (iarg != -1) {
    iarg = getopt_long(argc, argv, "d:c:f:rD", longopts, &index);
    switch (iarg) {
      case 'd': options.delay = atoi(optarg);
        break;

      case 'c': options.cmd = optarg;
        break;

      case 'f': options.font = optarg;
        break;

      case 'r': options.randomize = true;
        break;

      case 'D': options.debug = true;
        break;

      default: break;
    }
  }

  return options;
}
}
