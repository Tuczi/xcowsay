//
// Created by tuczi on 04.06.16.
//

#include "csi.hpp"

char *find_next_code(char *str) {
    while (*str && *str != 'm') {
        if (*str == ';') return str + 1;
        str++;
    }

    return NULL;
}

uint32_t get_extended_color(char *&next) {
    next = find_next_code(next);
    int code = atoi(next);
    uint32_t color;

    if (code == 5) {
        next = find_next_code(next);
        code = atoi(next);

        color = xterm_colors[code];
    } else if (code == 2) {
        next = find_next_code(next);
        int r = atoi(next);
        next = find_next_code(next);
        int g = atoi(next);
        next = find_next_code(next);
        int b = atoi(next);

        color = (r << 16) | (g << 8) | b;
    }

    return color;
}

// TODO buf (in consequences line) can contains partial esc sequence!
CSI_t get_color(std::string &str, size_t start) {
    CSI_t csi;
    char *next = (char *) str.c_str() + start + 2;

    while (next) {
        int code = atoi(next);
        if (code == 0) {
            csi = CSI_t();
        } else if (code == 1) {
            csi.bold = 1;
        } else if (code >= 30 && code <= 37) {
            csi.fg_color = xterm_colors[code - 30];
        } else if (code == 38) {
            csi.fg_color = get_extended_color(next);
        } else if (code == 39) {
            csi.fg_color = CSI_t().fg_color;
        } else if (code >= 40 && code <= 47) {
            csi.bg_color = xterm_colors[code - 40];
        } else if (code == 48) {
            csi.bg_color = get_extended_color(next);
        } else if (code == 49) {
            csi.bg_color = CSI_t().bg_color;
        }

        next = find_next_code(next);
    }

    return csi;
}