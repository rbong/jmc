#ifdef COVER
SDL_Surface *get_cover (struct mpd_song *song);
#endif

#ifndef COVER
#define COVER
#include "cover.c"
#endif
