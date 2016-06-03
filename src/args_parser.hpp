#ifndef ARGS_PARSER_HPP_
#define ARGS_PARSER_HPP_

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define no_argument 0
#define required_argument 1
#define optional_argument 2

typedef struct  {
  int delay;
  char* cmd;
} option_t;

option_t options = {0, 0};

const struct option longopts[] =
  {
    {"cmd",      required_argument,        0, 'c'},
    {"delay",    required_argument,        0, 'd'},
    {0,0,0,0},
  };


void set_options(int argc, char * argv[]) {
    int index;
    int iarg=0;
    int len;

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
          len = strlen(optarg) + 1;
          options.cmd = (char*) malloc(sizeof(char)*len);
          options.cmd[len-1] = '\0';
          strcpy(options.cmd, optarg);
          break;
      }
    }
}

#endif
