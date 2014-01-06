CFLAGS=-std=gnu99 -O3 -Wall
all: src/jmc.c -lmpdclient -lSDL -lSDL_image -lSDL_gfx
	gcc src/jmc.c -lmpdclient -lSDL -lSDL_image -lSDL_gfx $(CFLAGS) -o jmc
clean:
	rm -rf jmc
install:
	cp jmc /usr/bin/jmc
uninstall:
	rm /usr/bin/jmc
