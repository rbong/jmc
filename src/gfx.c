#include "options.h" // globals
#include "gfx.h"
#include "sdl.h" // draw_surface_sdl scale_surface_sdl
#include "playlist.h" // playlist variable

bool next_is_nul = false;
bool prev_is_nul = false;

// internal function prototypes
void draw_classic_mode (SDL_Surface *);
void draw_one_mode (SDL_Surface *);
void rect (SDL_Surface *, int, int, int, int, int);
void alpha_mask (SDL_Surface *, int);
void draw_album (SDL_Surface *, SDL_Surface *, int, int, int);
void draw_flipped (SDL_Surface *, SDL_Surface *, int, int, int);
void draw_both (SDL_Surface *, SDL_Surface *, int, int, int);

void draw_gfx (SDL_Surface *sur)
{
    clear_gfx (sur);
    if (cur_pos < 0)
        return;
    if (one_mode)
    {
        draw_one_mode (sur);
        return;
    }
    draw_classic_mode (sur);
}

void clear_gfx (SDL_Surface *sur)
{
    SDL_FillRect (sur, NULL, bg_color);
}

void draw_classic_mode (SDL_Surface *sur) // take this as an example
{
    if (cur_pos - offset > play_len || cur_pos - offset < 0)
        return;
    int root_size, size, pad, y_off;
    album *root_album = playlist [cur_pos - offset];
    if (root_album == NULL)
        return;
    album *cur_album = root_album->prev;

    if (verbose)
        printf ("drawing\n");

    if (sur->w < sur->h)
        root_size = sur->w * root_size_opt;
    else
        root_size = sur->h * root_size_opt;
    if (root_size > max_size_opt)
        root_size = max_size_opt;
    size = size_opt * root_size;
    if (size > max_size_opt)
    {
        size = max_size_opt;
        root_size = size / size_opt;
    }
    pad = size * pad_opt;

    int root_offset = sur->w/2 - root_size/2;
    int prev_offset = root_offset - pad - size;
    int next_offset = root_offset + pad + root_size;

    y_off = y_off_opt * size;
    if (y_off * 2 > root_size - size)
        y_off = (root_size - size) / 2;


    prev_is_nul = false;
    for (int i = prev_offset;
         i + size > 0 && cur_album != NULL;
         i -= pad + size, cur_album = cur_album->prev)
    {
        if (cur_album->cover != NULL)
        {
            draw_both (cur_album->cover, sur, i, sur->h/2 - size/2, size);
        }
    }
    if (cur_album == NULL)
        prev_is_nul = true;
    cur_album = root_album->next;
    next_is_nul = false;
    for (int i = next_offset;
         i < sur->w && cur_album != NULL;
         i += pad + size, cur_album = cur_album->next)
    {
        if (cur_album->cover != NULL)
        {
            draw_both (cur_album->cover, sur, i, sur->h/2 - size/2, size);
        }
    }
    if (cur_album == NULL)
        next_is_nul = true;

    if (root_album->cover != NULL)
    {
        draw_flipped (root_album->cover, sur,
                root_offset, sur->h/2 - root_size/2 - y_off, root_size);
        alpha_mask (sur, sur->h/2 + size/2);
        draw_album (root_album->cover, sur,
                root_offset, sur->h/2 - root_size/2 - y_off, root_size);
    }
    else
        alpha_mask (sur, sur->h/2 + size/2);
}

void draw_one_mode (SDL_Surface *sur)
{
    if (cur_pos - offset > play_len || cur_pos - offset < 0)
        return;
    int size, y_off;
    album *a = playlist [cur_pos - offset];
    next_is_nul = false;
    prev_is_nul = false;
    if (a == NULL)
        return;
    if (a->next == NULL)
        next_is_nul = true;
    if (a->prev == NULL)
        prev_is_nul = true;
    if (a->cover == NULL)
        return;

    if (verbose)
        printf ("drawing\n");

    if (sur->w < sur->h)
        size = sur->w * root_size_opt;
    else
        size = sur->h * root_size_opt;
    if (size > max_size_opt)
        size = max_size_opt;

    int off = sur->w/2 - size/2;

    y_off = y_off_opt * size;
    if (y_off * 2 > size - size)
        y_off = (size - size) / 2;

    draw_album (a->cover, sur, off, sur->h/2 - size/2 - y_off, size);
    draw_flipped (a->cover, sur, off, sur->h/2 - size/2 - y_off, size);
    alpha_mask (sur, sur->h/2 + size/2);
}

void rect (SDL_Surface *sur, int x, int y, int w, int h, int color)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_FillRect (sur, &rect, color);
}

void alpha_mask (SDL_Surface *sur, int y)
{
    SDL_Surface *alpha = SDL_CreateRGBSurface
        (SDL_SWSURFACE | SDL_ANYFORMAT, sur->w, sur->h, screen_bpp,
         0, 0, 0, 0);
    SDL_FillRect (alpha, NULL, bg_color);
    SDL_SetAlpha (alpha, SDL_SRCALPHA, trans);

    draw_surface_sdl (alpha, sur, 0, y);

    SDL_FreeSurface (alpha);
}

void draw_album (SDL_Surface *src, SDL_Surface *dst, int x, int y, int px)
{
    SDL_Surface *scaled = scale_surface_sdl (src, px, px);

    rect (dst, x, y, px, px, 0);
    draw_surface_sdl (scaled, dst, x, y);

    SDL_FreeSurface (scaled);
}

void draw_flipped (SDL_Surface *src, SDL_Surface *dst, int x, int y, int px)
{
    SDL_Surface *flipped = scale_surface_sdl (src, px, px * -1);

    rect (dst, x, y + px, px, px, 0);
    draw_surface_sdl (flipped, dst, x, y + px);

    SDL_FreeSurface (flipped);
}

void draw_both (SDL_Surface *src, SDL_Surface *dst, int x, int y, int px)
{
    draw_album (src, dst, x, y, px);
    draw_flipped (src, dst, x, y, px);
}
