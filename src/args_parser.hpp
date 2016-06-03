#ifndef ARGS_PARSER_HPP_
#define ARGS_PARSER_HPP_

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#define no_argument 0
#define required_argument 1
#define optional_argument 2

struct option_t {
  int delay;
  std::string cmd;
};

option_t options;

const struct option longopts[] =
  {
    {"cmd",      required_argument,        0, 'c'},
    {"delay",    required_argument,        0, 'd'},
    {0,0,0,0},
  };

void set_options(int argc, char * argv[]) {
  int index;
  int iarg=0;

  //turn off getopt error message
  opterr=1; 

  while(iarg != -1)
  {
    iarg = getopt_long(argc, argv, "d:c:", longopts, &index);

    switch (iarg)
    {
      case 'd':
        options.delay = atoi(optarg);
        break;

      case 'c':
        options.cmd = optarg;
        break;
    }
  }
}

#endif
