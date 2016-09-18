//
// Created by tuczi on 04.06.16.
//

#include "args_parser.hpp"

option_t get_options(int argc, char *argv[]) {
    option_t options;
    int index;
    int iarg = 0;

    //turn off getopt error message
    opterr = 1;

    while (iarg != -1) {
        iarg = getopt_long(argc, argv, "d:c:f:", longopts, &index);

        switch (iarg) {
            case 'd':
                options.delay = atoi(optarg);
                break;

            case 'c':
                options.cmd = optarg;
                break;

            case 'f':
                options.font = optarg;

            default:
                break;
        }
    }

    return options;
}
