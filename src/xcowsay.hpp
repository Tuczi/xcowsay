//
// Created by tuczi on 04.06.16.
//

#ifndef XCOWSAY_XCOWSAY_HPP
#define XCOWSAY_XCOWSAY_HPP

#include <cstring>
#include <vector>
#include <X11/Xlib.h>

#include "csi.hpp"
#include "args_parser.hpp"

constexpr size_t BUF_SIZE = 1024;

struct parsed_line_t {
    CSI_t color;
    const char *str;
    size_t len;

    parsed_line_t(CSI_t color_, const char* str_, int len_) : color(color_), str(str_), len(len_) { }
};

std::vector<parsed_line_t> parse_line(std::string &str, CSI_t last_color);

void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, XFontStruct *fs);

#endif //XCOWSAY_XCOWSAY_HPP
