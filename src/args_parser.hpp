//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_ARGS_PARSER_HPP
#define XCOWSAY_ARGS_PARSER_HPP

#include <getopt.h>
#include <string>
namespace xcowsay {

struct Options {
	int delay;
	std::string cmd;
	std::string font;

	bool debug = false;
};

class OptionsFactory {
	private:
		static const struct option longopts[];

	public:
		static Options fromArgs(int argc, char *argv[]);
};

}
#endif //XCOWSAY_ARGS_PARSER_HPP
