CC     = gcc
LIBS   = -lmpdclient -lSDL -lSDL_image -lm
OSXLIBS= -lmpdclient -lSDL -lSDL_image -lm -lSDL_gfx -lSDLmain -framework cocoa
CFLAGS = -std=gnu99 -Wall
DBUGFL = -g

SOURCES = \
	src/id3v2lib/frame.c \
	src/id3v2lib/header.c \
	src/id3v2lib/types.c \
	src/id3v2lib/utils.c \
	src/id3v2lib/id3v2lib.c \
	src/sdl_gfx/SDL_rotozoom.c \
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
osx: src/jmc.c
	$(CC) $(SOURCES) $(OSXLIBS) $(CFLAGS) -o jmc
debug: src/jmc.c
	$(CC) $(SOURCES) $(LIBS) $(CFLAGS) $(DBUGFL) -o jmc
clean:
	rm -rf jmc
install:
	cp jmc /usr/bin/jmc
uninstall:
	rm /usr/bin/jmc
