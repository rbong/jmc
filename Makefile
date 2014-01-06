CC     = gcc
LIBS   = -lmpdclient -lSDL -lSDL_image -lSDL_gfx 
CFLAGS = -std=gnu99 -O3 -Wall

SOURCES = \
	src/id3v2lib/frame.c \
	src/id3v2lib/header.c \
	src/id3v2lib/types.c \
	src/id3v2lib/utils.c \
	src/id3v2lib/id3v2lib.c \
	src/album.c \
	src/cover.c \
	src/gfx.c \
	src/mpd.c \
	src/options.c \
	src/playlist.c \
	src/sdl.c \
	src/jmc.c


all: src/jmc.c
	$(CC) $(SOURCES) $(LIBS) $(CFLAGS) -o jmc
clean:
	rm -rf jmc
install:
	cp jmc /usr/bin/jmc
uninstall:
	rm /usr/bin/jmc
