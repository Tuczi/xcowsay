#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include "vroot.h"

#define COMMAND "fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n"
#define SLEEP_IN_SEC 15

void draw(Display *dpy, Window root, XWindowAttributes wa, GC g, const int lineHeight) {
	int posX = random()%(wa.width/2), posY = random()%(wa.height/2) + 10;
	FILE *pp = popen(COMMAND, "r");
	if (!pp) return;

	while(1) {
		char buf[1000];
		char* line = fgets(buf, sizeof(buf), pp);
		
		if (!line) break;
		XDrawString(dpy, root, g, posX, (posY+=lineHeight), line, strlen(line)-1);
	}

	pclose(pp);
}

int main()
{
	Display *dpy;
	Window root;
	XWindowAttributes wa;
	GC g;

	XColor redx, reds;

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

	/* allocate the red color */
	XAllocNamedColor(dpy, DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy)),
		"red", &reds, &redx);

	/* set foreground color */
	XSetForeground(dpy, g, reds.pixel);

	/* load a font */
	f=XLoadFont(dpy, "fixed");
	XSetFont(dpy, g, f);
	
	/* get font metrics */
	XGetGCValues (dpy, g, GCFont, &v);
	fs = XQueryFont (dpy, v.font);

	/* get line height */
	int lineHeight = fs ? fs->ascent+fs->descent : 13;

	/* draw something */
	while(1)
	{
		XClearWindow(dpy, root);

		draw(dpy, root, wa, g, lineHeight);

		/* flush changes and sleep */
		XFlush(dpy);
		sleep(SLEEP_IN_SEC);
	}

	XCloseDisplay(dpy);
}
