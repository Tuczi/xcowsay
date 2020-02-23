//
// Created by tuczi on 04.06.16.
//

#include "args_parser.hpp"

namespace xcowsay {

const struct option OptionsFactory::longopts[] = {
	{"cmd",   required_argument, nullptr, 'c'},
	{"delay", required_argument, nullptr, 'd'},
	{"font",  required_argument, nullptr, 'f'},
	{"debug", no_argument, nullptr, 'D'},
	{nullptr, no_argument,  nullptr, '\0'},
};

Options OptionsFactory::fromArgs(int argc, char* argv[]) {
	Options options;
	int index;
	int iarg = 0;

	//turn off getopt error message
	opterr = 1;

	while (iarg != -1) {
		iarg = getopt_long(argc, argv, "d:c:f:D", longopts, &index);
		switch (iarg) {
			case 'd':
				options.delay = atoi(optarg);
				break;

			case 'c':
				options.cmd = optarg;
				break;

			case 'f':
				options.font = optarg;
				break;

			case 'D':
				options.debug = true;
				break;

			default:
				break;
		}
	}

	return options;
}

}
