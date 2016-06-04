//
// Created by tuczi on 04.06.16.
//

#include "xcowsay.hpp"

std::vector<parsed_line_t> parse_line(std::string &str, CSI_t last_color) {
    std::vector<parsed_line_t> parsed_line;

    //first part
    size_t pos = str.find("\x1B[");
    parsed_line.push_back(parsed_line_t(last_color, str.c_str(), str.size()));

    //other parts
    while (pos != std::string::npos) {
        size_t prefix_end = str.find('m', pos) + 1;

        auto &last = parsed_line.back();
        last.len = str.c_str() + pos - last.str;

        parsed_line.push_back(parsed_line_t(get_color(str, pos), str.c_str() + prefix_end, 0));
        pos = str.find("\x1B[", prefix_end);
    }

    auto &last = parsed_line.back();
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

        char *line = fgets((char *) buf.c_str(), BUF_SIZE, pp);
        if (!line) break;

        size_t len = strnlen(buf.c_str(), BUF_SIZE);
        buf.resize(len - 1);

        auto parsed_line = parse_line(buf, last_color);
        for (auto &fragment: parsed_line) {
            XSetForeground(dpy, g, fragment.color.fg_color);
            XDrawString(dpy, root, g, posXTmp, posY, fragment.str, fragment.len);
            posXTmp += XTextWidth(fs, fragment.str, fragment.len);
        }
        last_color = parsed_line.back().color;

        posY += lineHeight;
    }

    pclose(pp);
}
