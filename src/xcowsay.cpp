#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <vector>

#include "args_parser.hpp"
#include "xterm_collors.hpp"
#include "vroot.h"


#define BUF_SIZE 1024


//TODO make bold
struct CSI_t {
  bool bold;
  uint32_t fg_color;
  uint32_t bg_color;
  
  CSI_t(): bold(false), fg_color(0xFFFFFF), bg_color(0) { }
  
  CSI_t(uint32_t fg_color, uint32_t bg_color): bold(false), fg_color(fg_color), bg_color(bg_color) { }
};

//TODO refactor
struct parsed_line_t {
  CSI_t color;
  const char *str;
  size_t len;
  
  parsed_line_t(CSI_t color, const char *str, int len): color(color), str(str), len(len) { }
};

char* find_next_code(char* str) {
  while(*str && *str!='m'){
    if(*str==';') return str+1;
    str++;
  }

  return NULL;
}

// TODO refactor (reusable code for csi code 38 and 48)
// TODO buf (in consequences line) can contains partial esc sequence!
CSI_t get_color(const char *str) {
    CSI_t csi;
    str += 2;
    char* next = (char*) str;
    while (next != NULL) {
        int code = atoi(next);
        if(code == 0) {
            csi = CSI_t();
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
            csi.fg_color = CSI_t().fg_color;
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
            csi.bg_color = CSI_t().bg_color;
        }

        next = find_next_code(next);
    }

    return csi;
}

std::vector<parsed_line_t> parse_line(std::string& str, CSI_t last_color) {
  std::vector<parsed_line_t> parsed_line;

  //first part
  size_t pos = str.find("\e[");
  parsed_line.push_back(parsed_line_t(last_color, str.c_str(), str.size()));
  
  //other parts
  while(pos != std::string::npos) {
    const char* substr = str.c_str() + pos;
    size_t prefix_end = str.find('m', pos) + 1;
    
    auto& last = parsed_line.back();
    last.len = substr - last.str;
    
    parsed_line.push_back(parsed_line_t(get_color(substr), str.c_str() + prefix_end , 0));
    pos = str.find("\e[", prefix_end);
  }

  auto& last = parsed_line.back();
  last.len = str.c_str() + str.size() - last.str;

  return parsed_line;
}


void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, XFontStruct *fs) {
    int posX = random() % (wa.width / 2), posY = random() % (wa.height / 2) + 10;
    FILE *pp = popen(options.cmd.c_str(), "r");
    if (!pp) return;

    /* get line height */
    int lineHeight = fs ? fs->ascent + fs->descent : 13;
    CSI_t last_color;
    
    while (true) {
        std::string buf(BUF_SIZE, '\0');
        int posXTmp = posX;

        char *line = fgets((char*)buf.c_str(), BUF_SIZE, pp);
        if (!line) break;
        
        size_t len = strnlen(buf.c_str(), BUF_SIZE);
        buf.resize(len-1);
        
        auto parsed_line = parse_line(buf, last_color);
        for(auto& fragment: parsed_line) {
            XSetForeground(dpy, g, fragment.color.fg_color);
            XDrawString(dpy, root, g, posXTmp, posY, fragment.str, fragment.len);
            posXTmp += XTextWidth(fs, fragment.str, fragment.len);
        }
        last_color = parsed_line.back().color;

        posY += lineHeight;
    }

    pclose(pp);
}
