#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include "vroot.h"

#define COMMAND "fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n"
#define SLEEP_IN_SEC 15
#define PIXELS_SIZE 6
#define STRIP_LEN 4
#define BUF_SIZE 1024

void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, XFontStruct* fs) {
	int posX = random()%(wa.width/2), posY = random()%(wa.height/2) + 10;
	FILE *pp = popen(COMMAND, "r");
	if (!pp) return;

	//red, orange, yellow, green, blue, violet
	unsigned long pixels[PIXELS_SIZE]= {0xFF0000, 0xFFA500, 0xFFFF00, 0x00A500, 0x0000A5, 0xA500A5};
	/* get line height */
	int lineHeight = fs ? fs->ascent+fs->descent : 13;

	while(1) {
		char buf[BUF_SIZE];
		char* line = fgets(buf, sizeof(buf), pp);

		if (!line) break;

		int i=0, len = strlen(line)-1;
		/* set up end of string */
		for(i=len; i<len+STRIP_LEN && i<sizeof(buf); i++) 
			line[i] = ' ';

		int posXTmp = posX;

		for(i=0; i<len; i+=STRIP_LEN) {
			XSetForeground(dpy, g, pixels[(i/STRIP_LEN)%PIXELS_SIZE]);

			XDrawString(dpy, root, g, posXTmp, posY, line+i, STRIP_LEN);
			posXTmp += XTextWidth(fs, line+i, STRIP_LEN);
		}

		posY += lineHeight;
	}

	pclose(pp);
}

int main()
{
	Display *dpy;
	Window root;
	XWindowAttributes wa;
	GC g;

	Font f;
	XFontStruct *fs;
	XGCValues v;

	/* open the display (connect to the X server) */
	dpy = XOpenDisplay(getenv ("DISPLAY"));

	/* get the root window */
	root = DefaultRootWindow(dpy);

	/* get attributes of the root window */
	XGetWindowAttributes(dpy, root, &wa);

	/* create a GC for drawing in the window */
	g = XCreateGC(dpy, root, 0, NULL);

	/* load a font */
	f=XLoadFont(dpy, "fixed");
	XSetFont(dpy, g, f);
	
	/* get font metrics */
	XGetGCValues (dpy, g, GCFont, &v);
	fs = XQueryFont (dpy, v.font);

	/* draw something */
	while(1)
	{
		XClearWindow(dpy, root);

		draw(dpy, root, wa, g, fs);

		/* flush changes and sleep */
		XFlush(dpy);
		sleep(SLEEP_IN_SEC);
	}

	XCloseDisplay(dpy);
}
