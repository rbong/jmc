#ifndef SDL
#define SDL

#include <SDL/SDL.h>

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;

int poll_sdl (void);
void finish_sdl (void);
void wait_sdl (void);
SDL_Surface *load_img_sdl (char *, int);
SDL_Surface *scale_surface_sdl (SDL_Surface *, int, int);
int draw_surface_sdl (SDL_Surface *, SDL_Surface *, int, int);
SDL_Surface *blur_img_sdl (SDL_Surface *);

#endif

