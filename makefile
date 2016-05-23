all: build install install-extra-cows
CC=gcc
CFLAGS=-O3 -s
build:
	$(CC) $(CFLAGS) -o xcowsay xcowsay.c -L/usr/lib -lX11
install:
	cp xcowsay /usr/lib/xscreensaver
	cp xscreen-config/xcowsay.xml /usr/share/xscreensaver/config/xcowsay.xml
install-extra-cows:
	cp cows/* /usr/share/cows
