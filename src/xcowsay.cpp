#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include "args_parser.hpp"
#include "xterm_collors.hpp"
#include "vroot.h"


#define BUF_SIZE 1024


//TODO make bold
typedef struct {
    int bold; //0,1
    int fg_color; //0-255
    int bg_color; //0-255
} CSI_t;

CSI_t csi_reset() {
    CSI_t csi = {0, 0xffffff, 0};
    return csi;
}

//TODO refactor
typedef struct {
    int parts_count;
    CSI_t *color;
    int *len;
    char **str; // array of pointers - free only char** str(first level)
} parsed_line_t;

void free_parsed_line(parsed_line_t *parsed_line) {
    free(parsed_line->color);
    free(parsed_line->len);
    free(parsed_line->str);

    parsed_line->color = NULL;
    parsed_line->len = NULL;
    parsed_line->str = NULL;
    parsed_line->parts_count = 0;
}

char* find_next_code(char* str) {
	while(*str && *str!='m'){
		if(*str==';') return str+1;
		str++;
	}
	
	return NULL;
}

// TODO refactor (reusable code for csi code 38 and 48)
// TODO buf (in consequences line) can contains partial esc sequence!
CSI_t get_color(char *str) {
    CSI_t csi = csi_reset();
    str += 2;
    char* next = str;
    while (next != NULL) {
        int code = atoi(next);
        if(code == 0) {
            csi = csi_reset();
        } else if(code == 1) {
            csi.bold = 1;
        } else if(code >=30 && code <=37) {
            csi.fg_color = xterm_colors[code-30];
        } else if(code == 38) {
            next = find_next_code(next);
            code = atoi(next);
            if(code == 5) {
                next = find_next_code(next);
                code = atoi(next);
                
                csi.fg_color = xterm_colors[code];
            } else if(code == 2) {
                next = find_next_code(next);
                int r = atoi(next);
                str = next+1;
                next = find_next_code(next);
                int g = atoi(next);
                str = next+1;
                next = find_next_code(next);
                int b = atoi(next);
                
                csi.fg_color = (r<<16) | (g<<8) | b;
            }
        } else if(code == 39) {
            csi.fg_color = csi_reset().fg_color;
        } else if(code >=40 && code <=47) {
            csi.bg_color = xterm_colors[code-40];
        } else if(code == 48) {
            next = find_next_code(next);
            code = atoi(next);
            if(code == 5) {
                next = find_next_code(next);
                code = atoi(next);
                
                csi.bg_color = xterm_colors[code];
            } else if(code == 2) {
                next = find_next_code(next);
                int r = atoi(next);
                str = next+1;
                next = find_next_code(next);
                int g = atoi(next);
                str = next+1;
                next = find_next_code(next);
                int b = atoi(next);
                
                csi.bg_color = (r<<16) | (g<<8) | b;
            }
        } else if(code == 49) {
            csi.bg_color = csi_reset().bg_color;
        }

        next = find_next_code(next);
    }

    return csi;
}

int get_prefix_len(char *str) {
    char *result = strstr(str, "m");
    if (result)
        return result - str + 1;
    return 0;
}

parsed_line_t parse_line(char *init_str, int init_len, CSI_t last_color) {
    parsed_line_t parsed_line;
    parsed_line.parts_count = 1;

    char *substr = init_str - 1;

    while (substr < init_str + init_len && (substr = strstr(substr + 1, "\e[")))
        parsed_line.parts_count++;

    parsed_line.color = (CSI_t*)malloc(parsed_line.parts_count * sizeof(*parsed_line.color));
    parsed_line.len = (int*) malloc(parsed_line.parts_count * sizeof(*parsed_line.len));
    parsed_line.str = (char**) malloc(parsed_line.parts_count * sizeof(*parsed_line.str));

    //first part:
    substr = strstr(init_str, "\e[");
    parsed_line.color[0] = last_color;
    parsed_line.str[0] = init_str;
    parsed_line.len[0] = substr - init_str;

    //other parts:
    substr = substr - 1;
    for (int i = 1; i < parsed_line.parts_count; i++) {
        substr = strstr(substr + 1, "\e[");
        int prefix_len = get_prefix_len(substr);

        parsed_line.color[i] = get_color(substr);
        parsed_line.str[i] = substr + prefix_len;
        parsed_line.len[i - 1] = substr - parsed_line.str[i - 1];
    }

    parsed_line.len[parsed_line.parts_count - 1] = init_str + init_len - parsed_line.str[parsed_line.parts_count - 1];

    return parsed_line;
}

void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, XFontStruct *fs) {
    int posX = random() % (wa.width / 2), posY = random() % (wa.height / 2) + 10;
    FILE *pp = popen(options.cmd, "r");
    if (!pp) return;

    /* get line height */
    int lineHeight = fs ? fs->ascent + fs->descent : 13;
    CSI_t last_color = csi_reset();
    
    while (1) {
        char buf[BUF_SIZE] ={' '};
        char *line = fgets(buf, sizeof(buf), pp);

        if (!line) break;

        int i = 0, len = strlen(line) - 1;
        /* set up end of string */
        int part_len = 4;
        for (i = len; i < len + part_len && i < sizeof(buf); i++)
            line[i] = ' ';

        int posXTmp = posX;

        parsed_line_t parsed_line = parse_line(buf, len, last_color);
        for (i = 0; i < parsed_line.parts_count; i++) {
            XSetForeground(dpy, g, parsed_line. color[i].fg_color);
            XDrawString(dpy, root, g, posXTmp, posY, parsed_line.str[i], parsed_line.len[i]);

            posXTmp += XTextWidth(fs, parsed_line.str[i], parsed_line.len[i]);
        }

        last_color = parsed_line.color[parsed_line.parts_count - 1];
        free_parsed_line(&parsed_line);

        posY += lineHeight;
    }

    pclose(pp);
}
