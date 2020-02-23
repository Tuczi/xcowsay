//
// Created by tuczi on 04.06.16.
//

#include "csi.hpp"

namespace xcowsay {

/**
 * returns
 *  valid CSI 24-bit color if success
 * 	-1 if cannot parse
 **/
int CsiParser::getExtendedColor(const char* string, char** end) {
	//TODO what if previus character was not ';'?
	int code = strToCsiInt(string, end);
	if(code == -1) {
		 return -1;
	}

	if (code == 5) {
		code = strToCsiInt(string+2, end);
		if(code == -1) {
			return -1;
		}

		return xterm_colors[code];
	} else if (code == 2) { 
		int r = strToCsiInt(string+2, end);
		if(code == -1) {
			return -1;
		}
		
		int g = strToCsiInt((*end)+1, end);
		if(code == -1) {
			return -1;
		}

		int b = strToCsiInt((*end)+1, end);
		if(code == -1) {
			return -1;
		}

		return (r << 16) | (g << 8) | b;
	}

	//TODO handle unexpected code error
	return -1;
}

/**
 * set "end" pointer to the end of CSI code
 * 
 * returns
 *  valid CSI code if success
 * 	-1 if cannot parse
 **/
int CsiParser::strToCsiInt(const char* string, char** end) {
	const long csiCodeCandidate = std::strtol(string, end, 10);

	if ((*end == nullptr)
		|| ((**end != ';') && (**end != 'm'))
		|| (errno == ERANGE) //TODO != 0 ?
		|| (csiCodeCandidate > 255) //TODO MAX_CSI_CODE_VALUE
		|| (csiCodeCandidate < 0)) {//TODO MIN_CSI_CODE_VALUE

		return -1;
	}

	return csiCodeCandidate;
}

/**
 * Returns parsed CSI or partially parsed CSI (TODO with pointer where not fully processed code starts).
 * 
 * Partial CSI code - it might happen that buffer ends in the middle of the CSI sequence. Sencarios
 * 1. Method is not able to read code (e.g. buffer constians only "3" instead of "35;" or "35m")
 * 2. Method is not able to read extended colors codes (e.g. buffer contains "38;2;2" instead of "38;2;201;" or "38;2;202;")
 **/
Csi CsiParser::parseCsiSequence(Csi csi, char* start, char** end) {
	char* next = start;

	while (true) {
		int code = strToCsiInt(next, end);
		if(code == -1) {
			//TODO return partial Csi
			return csi;
		}

		if (code == 0) {
			csi = Csi();
		} else if (code == 1) {
			csi.bold = 1;
		} else if (code >= 30 && code <= 37) {
			csi.fg_color = xterm_colors[code - 30];
		} else if (code == 38) {
			const int colorCandidate = getExtendedColor(next+3, end);
			if(colorCandidate == -1) {
				//TODO return partial Csi
				return csi;
			}

			csi.fg_color = colorCandidate;
		} else if (code == 39) {
			csi.fg_color = Csi().fg_color;
		} else if (code >= 40 && code <= 47) {
			csi.bg_color = xterm_colors[code - 40];//TODO error check
		} else if (code == 48) {
			const int colorCandidate = getExtendedColor(next+3, end);
			if(colorCandidate == -1) {
				//TODO return partial Csi
				return csi;
			}

			csi.bg_color = colorCandidate;
		} else if (code == 49) {
			csi.bg_color = Csi().bg_color;
		}

		next = *end +1;
		if(*next == '\0') {
			//TODO return partial Csi
			return csi;
		}
		if(*next == 'm') {
			return csi;
		}
	}

	return csi;
}

void CsiParser::moveBuffer(std::string&& stringBuffer) {
	readStart = 0;
	bufferEnd = false;

	buffer = stringBuffer;
}

void CsiParser::parseNextFragment() {
	if(!previousBufferCsiStartSequence.empty()) {
		size_t csiEnd = buffer.find('m');
		if (csiEnd == std::string::npos) {// CSI sequence started in previous buffer does not end in current buffer
			/**
			 * TODO fix potential OOM in case of looping
			 * Looks like in worst case (extended code has been used) we need to pass forward only 16 characters.
			 * e.g. extended code "38;2;201;202;203" - set fg color to r=201,g=202,b=203
			 * 
			 * We can easly parse part of CSI sequence and then memorize the state in currentFragment.
			 * Then we can update it state while processing next buffer.
			 * 
			 * We can repeat the algorithm if CSI sequence started in previous buffer does not end in current buffer (looping without OOM).
			 * 
			 * TODO:
			 * 1. get_color should be able to return partial state
			 * 2. get_color should be able to notify if it is not bale to return partial state (end of buffer before reading all collors).
			 *   - consider also scenario when buffer constians only "3" instead of e.g. "35"
			 **/
			previousBufferCsiStartSequence += buffer;

			bufferEnd = true;
			return;
		}

		previousBufferCsiStartSequence += buffer.substr(0, csiEnd+1);

		char* end;
		currentFragment = CsiStringFragment(parseCsiSequence(currentFragment.color, (char*) previousBufferCsiStartSequence.c_str()+2, &end), buffer.c_str() + csiEnd +1, 0);

		readStart = csiEnd +1;
		previousBufferCsiStartSequence = "";
		return;
	}

	size_t csiStart = buffer.find("\x1B[", readStart); // find CSI start sequence
	//TODO if '\x1B'('\e') is in the end of buffer and '[' is in the next buffer

	if (csiStart == std::string::npos) { // buffer.substr(readStart) is simple string - set current fragment to buffer.substr(readStart)
		currentFragment = CsiStringFragment(currentFragment.color, buffer.c_str()+readStart, buffer.size()-readStart);

		bufferEnd = true;
		return;
	}

	if(csiStart != readStart) {// buffer has some text before next CSI start sequence - set current fragment to buffer.substr(readStart, csiStart-readStart)
		currentFragment = CsiStringFragment(currentFragment.color, buffer.c_str()+readStart, csiStart-readStart);

		readStart = csiStart;
		return;
	}

	// buffer.substr(readStart) starts with new CSI sequence
	size_t csiEnd = buffer.find('m', csiStart);
	if (csiEnd == std::string::npos) { //buffer.substr(readStart) does not contain CSI end character (CSI continuation is in a next buffer) - save partial CSI sequence
		previousBufferCsiStartSequence = buffer.substr(csiStart);

		bufferEnd = true;
		return;
	}

	// buffer.substr(readStart) starts with new CSI sequence and contains end of the sequance - buffer.substr(csiStart+1, csiEnd-csiStart) is the CSI sequence
	char* end;
	currentFragment = CsiStringFragment(parseCsiSequence(currentFragment.color, (char*) buffer.c_str()+csiStart+2, &end), buffer.c_str() + csiEnd + 1, 0);
	readStart = csiEnd +1;
}

/**
 * returns:
 *   true if has next fragment to pare (buffer has not been fully read)
 *   false otherwise
 **/
bool CsiParser::hasNextFragment() {
	return !bufferEnd;
}

CsiStringFragment CsiParser::getCurrentStringFragment() {
	return currentFragment;
}

}
