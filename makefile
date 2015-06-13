all: build install

build:
	gcc -o xcowsay xcowsay.c -L/usr/lib -lX11
install:
	cp xcowsay /usr/lib/xscreensaver
