//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_CSI_HPP
#define XCOWSAY_CSI_HPP

#include "xterm_colors.hpp"
#include <string>

//TODO implement bold
struct CSI_t {
    bool bold;
    uint32_t fg_color;
    uint32_t bg_color;

    CSI_t() : bold(false), fg_color(0xFFFFFF), bg_color(0) { }

    CSI_t(uint32_t fg_color_, uint32_t bg_color_) : bold(false), fg_color(fg_color_), bg_color(bg_color_) { }
};

char *find_next_code(char *str);

uint32_t get_extended_color(char *&next);

CSI_t get_color(std::string &str, size_t start);

#endif //XCOWSAY_CSI_HPP
