//
// Created by tuczi on 04.06.16.
//

#include "args_parser.hpp"

option_t options;

void set_options(int argc, char *argv[]) {
    int index;
    int iarg = 0;

    //turn off getopt error message
    opterr = 1;

    while (iarg != -1) {
        iarg = getopt_long(argc, argv, "d:c:", longopts, &index);

        switch (iarg) {
            case 'd':
                options.delay = atoi(optarg);
                break;

            case 'c':
                options.cmd = optarg;
                break;
        }
    }
}