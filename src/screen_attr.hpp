//
// Created by tuczi on 18.09.16.
//

#ifndef XCOWSAY_SCREEN_ATTR_HPP
#define XCOWSAY_SCREEN_ATTR_HPP

#include <vector>

#include "vroot.h"
#include "args_parser.hpp"

struct x_screen_attr_t {
    XWindowAttributes wa;
    Window root;
    GC g;
    XFontStruct *fs;
};

std::vector<x_screen_attr_t> get_screens_attr(Display *dpy, option_t options);

#endif //XCOWSAY_SCREEN_ATTR_HPP
