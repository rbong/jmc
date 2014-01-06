#ifdef SDL
int poll_sdl (void);
void finish_sdl (void);
void wait_sdl (void);
SDL_Surface *load_img_sdl (char *, int);
SDL_Surface *scale_surface_sdl (SDL_Surface *, int, int);
int draw_surface_sdl (SDL_Surface *, SDL_Surface *, int, int);
#endif

#ifndef SDL
#define SDL
#include "sdl.c"
#endif
