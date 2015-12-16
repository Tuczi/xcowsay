all: build install
CFLAGS=-O3 -s
build:
	gcc $(CFLAGS) -o xcowsay xcowsay.c -L/usr/lib -lX11
install:
	cp xcowsay /usr/lib/xscreensaver
