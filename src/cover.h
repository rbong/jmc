#ifndef COVER
#define COVER

#include <SDL/SDL.h>
#include "mpd.h"

SDL_Surface *get_cover (struct mpd_song *song);

#endif

