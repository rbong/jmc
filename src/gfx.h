#ifdef GFX
extern bool next_is_nul;
extern bool prev_is_nul;

void draw_gfx (SDL_Surface *);
void clear_gfx (SDL_Surface *);
#endif

#ifndef GFX
#define GFX
#include "gfx.c"
#endif
