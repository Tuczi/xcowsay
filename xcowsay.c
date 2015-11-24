#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include "vroot.h"

#define COMMAND "fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term"
#define SLEEP_IN_SEC 15
#define COLORS_SIZE 256
#define BUF_SIZE 1024

int xterm_colors[COLORS_SIZE] = {
        0x000000, 0x800000, 0x008000, 0x808000, 0x000080,
        0x800080, 0x008080, 0xc0c0c0, 0x808080, 0xff0000,
        0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff,
        0xffffff, 0x000000, 0x00005f, 0x000087, 0x0000af,
        0x0000df, 0x0000ff, 0x005f00, 0x005f5f, 0x005f87,
        0x005faf, 0x005fdf, 0x005fff, 0x008700, 0x00875f,
        0x008787, 0x0087af, 0x0087df, 0x0087ff, 0x00af00,
        0x00af5f, 0x00af87, 0x00afaf, 0x00afdf, 0x00afff,
        0x00df00, 0x00df5f, 0x00df87, 0x00dfaf, 0x00dfdf,
        0x00dfff, 0x00ff00, 0x00ff5f, 0x00ff87, 0x00ffaf,
        0x00ffdf, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087,
        0x5f00af, 0x5f00df, 0x5f00ff, 0x5f5f00, 0x5f5f5f,
        0x5f5f87, 0x5f5faf, 0x5f5fdf, 0x5f5fff, 0x5f8700,
        0x5f875f, 0x5f8787, 0x5f87af, 0x5f87df, 0x5f87ff,
        0x5faf00, 0x5faf5f, 0x5faf87, 0x5fafaf, 0x5fafdf,
        0x5fafff, 0x5fdf00, 0x5fdf5f, 0x5fdf87, 0x5fdfaf,
        0x5fdfdf, 0x5fdfff, 0x5fff00, 0x5fff5f, 0x5fff87,
        0x5fffaf, 0x5fffdf, 0x5fffff, 0x870000, 0x87005f,
        0x870087, 0x8700af, 0x8700df, 0x8700ff, 0x875f00,
        0x875f5f, 0x875f87, 0x875faf, 0x875fdf, 0x875fff,
        0x878700, 0x87875f, 0x878787, 0x8787af, 0x8787df,
        0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf,
        0x87afdf, 0x87afff, 0x87df00, 0x87df5f, 0x87df87,
        0x87dfaf, 0x87dfdf, 0x87dfff, 0x87ff00, 0x87ff5f,
        0x87ff87, 0x87ffaf, 0x87ffdf, 0x87ffff, 0xaf0000,
        0xaf005f, 0xaf0087, 0xaf00af, 0xaf00df, 0xaf00ff,
        0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fdf,
        0xaf5fff, 0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af,
        0xaf87df, 0xaf87ff, 0xafaf00, 0xafaf5f, 0xafaf87,
        0xafafaf, 0xafafdf, 0xafafff, 0xafdf00, 0xafdf5f,
        0xafdf87, 0xafdfaf, 0xafdfdf, 0xafdfff, 0xafff00,
        0xafff5f, 0xafff87, 0xafffaf, 0xafffdf, 0xafffff,
        0xdf0000, 0xdf005f, 0xdf0087, 0xdf00af, 0xdf00df,
        0xdf00ff, 0xdf5f00, 0xdf5f5f, 0xdf5f87, 0xdf5faf,
        0xdf5fdf, 0xdf5fff, 0xdf8700, 0xdf875f, 0xdf8787,
        0xdf87af, 0xdf87df, 0xdf87ff, 0xdfaf00, 0xdfaf5f,
        0xdfaf87, 0xdfafaf, 0xdfafdf, 0xdfafff, 0xdfdf00,
        0xdfdf5f, 0xdfdf87, 0xdfdfaf, 0xdfdfdf, 0xdfdfff,
        0xdfff00, 0xdfff5f, 0xdfff87, 0xdfffaf, 0xdfffdf,
        0xdfffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
        0xff00df, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87,
        0xff5faf, 0xff5fdf, 0xff5fff, 0xff8700, 0xff875f,
        0xff8787, 0xff87af, 0xff87df, 0xff87ff, 0xffaf00,
        0xffaf5f, 0xffaf87, 0xffafaf, 0xffafdf, 0xffafff,
        0xffdf00, 0xffdf5f, 0xffdf87, 0xffdfaf, 0xffdfdf,
        0xffdfff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf,
        0xffffdf, 0xffffff, 0x080808, 0x121212, 0x1c1c1c,
        0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
        0x585858, 0x606060, 0x666666, 0x767676, 0x808080,
        0x8a8a8a, 0x949494, 0x9e9e9e, 0xa8a8a8, 0xb2b2b2,
        0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4,
        0xeeeeee};

typedef struct {
    int parts_count, *color, *len;
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

int get_color(char *str) {
    int color;
    if (sscanf(str, "\e[%*[0-9];%*[0-9];%*[0-9];%dm", &color) > 0
        || sscanf(str, "\e[%*[0-9];%*[0-9];%dm", &color) > 0
        || sscanf(str, "\e[%*[0-9];%dm", &color) > 0
        || sscanf(str, "\e[%dm", &color) > 0)
        return xterm_colors[color % COLORS_SIZE];
    return 0;
}

int get_prefix_len(char *str) {
    char *result = strstr(str, "m");
    if (result)
        return result - str + 1;
    return 0;
}

parsed_line_t parse_line(char *init_str, int init_len, int last_color) {
    parsed_line_t parsed_line;
    parsed_line.parts_count = 1;

    char *substr = init_str - 1;
    int color = -1;

    while (substr < init_str + init_len && (substr = strstr(substr + 1, "\e[")))
        parsed_line.parts_count++;

    parsed_line.color = malloc(parsed_line.parts_count * sizeof(parsed_line.color));
    parsed_line.len = malloc(parsed_line.parts_count * sizeof(parsed_line.len));
    parsed_line.str = malloc(parsed_line.parts_count * sizeof(parsed_line.str));

    //first part:
    substr = strstr(init_str, "\e[");
    parsed_line.color[0] = last_color;
    parsed_line.str[0] = init_str;
    parsed_line.len[0] = substr - init_str;

    //other parts:
    substr = substr - 1;
    for (int i = 1; i < parsed_line.parts_count; i++) {
        substr = strstr(substr + 1, "\e[");
        color = get_color(substr);
        int prefix_len = get_prefix_len(substr);

        parsed_line.color[i] = color;
        parsed_line.str[i] = substr + prefix_len;
        parsed_line.len[i - 1] = substr - parsed_line.str[i - 1];
    }

    parsed_line.len[parsed_line.parts_count - 1] = init_str + init_len - parsed_line.str[parsed_line.parts_count - 1];

    return parsed_line;
}

void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, XFontStruct *fs) {
    int posX = random() % (wa.width / 2), posY = random() % (wa.height / 2) + 10;
    FILE *pp = popen(COMMAND, "r");
    if (!pp) return;

    /* get line height */
    int lineHeight = fs ? fs->ascent + fs->descent : 13;
    int last_color = xterm_colors[15];
    while (1) {
        char buf[BUF_SIZE];
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
            XSetForeground(dpy, g, parsed_line.color[i]);
            XDrawString(dpy, root, g, posXTmp, posY, parsed_line.str[i], parsed_line.len[i]);

            posXTmp += XTextWidth(fs, parsed_line.str[i], parsed_line.len[i]);
        }

        last_color = parsed_line.color[parsed_line.parts_count - 1];
        free_parsed_line(&parsed_line);

        posY += lineHeight;
    }

    pclose(pp);
}

int main() {
    Display *dpy;
    Window root;
    XWindowAttributes wa;
    GC g;

    Font f;
    XFontStruct *fs;
    XGCValues v;

    /* open the display (connect to the X server) */
    dpy = XOpenDisplay(getenv("DISPLAY"));

    /* get the root window */
    root = DefaultRootWindow(dpy);

    /* get attributes of the root window */
    XGetWindowAttributes(dpy, root, &wa);

    /* create a GC for drawing in the window */
    g = XCreateGC(dpy, root, 0, NULL);

    /* load a font */
    f = XLoadFont(dpy, "fixed");
    XSetFont(dpy, g, f);

    /* get font metrics */
    XGetGCValues(dpy, g, GCFont, &v);
    fs = XQueryFont(dpy, v.font);

    /* draw something */
    while (1) {
        XClearWindow(dpy, root);

        draw(dpy, root, wa, g, fs);

        /* flush changes and sleep */
        XFlush(dpy);
        sleep(SLEEP_IN_SEC);
    }

    XCloseDisplay(dpy);
}
