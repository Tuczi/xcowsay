//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_CSI_HPP
#define XCOWSAY_CSI_HPP

#include "xterm_colors.hpp"
#include <string>

namespace xcowsay {

//TODO implement bold
struct Csi {
	bool bold;
	uint32_t fg_color;
	uint32_t bg_color;

	Csi() : bold(false), fg_color(0xFFFFFF), bg_color(0) { }

	Csi(uint32_t fg_color_, uint32_t bg_color_) : bold(false), fg_color(fg_color_), bg_color(bg_color_) { }
};

struct CsiStringFragment {
	Csi color;
	const char *str;
	size_t len;

	CsiStringFragment(Csi color_, const char* str_, int len_) : color(color_), str(str_), len(len_) { }
};

class CsiParser {
	private:
		CsiStringFragment currentFragment = CsiStringFragment(Csi(), nullptr, 0);// TODO fix passing nullptr
		std::string buffer;
		bool bufferEnd = false;
		size_t readStart = 0;
		std::string previousBufferCsiStartSequence;

		int getExtendedColor(const char*, char**);
		Csi parseCsiSequence(Csi, char*, char**);
		int strToCsiInt(const char*, char**);

	public:
		void moveBuffer(std::string&&);
		bool hasNextFragment();
		void parseNextFragment();
		CsiStringFragment getCurrentStringFragment();
};

}
#endif //XCOWSAY_CSI_HPP
