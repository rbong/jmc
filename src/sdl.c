#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h> // png support
#include <mpd/client.h> // mpd_*

#include "options.h" // globals
#include "sdl.h"
#include "gfx.h" // draw_gfx
#include "playlist.h" // playlist, offset

int poll_sdl (void)
{
    int w = 0, h = 0;
    SDL_Event event;

    while (SDL_PollEvent (&event) != 0)
    {
        if (event.type == SDL_QUIT)
            return 1;
        else if (event.type == SDL_KEYDOWN)
        {
            int status = -1;
            int key = event.key.keysym.sym;
            const SDL_VideoInfo *info;
            switch (key)
            {
                case SDLK_q:
                    return 1;
                    break;
                case SDLK_r:
                    info = SDL_GetVideoInfo ();
                    screen = SDL_SetVideoMode (info->current_w,
                            info->current_h, screen_bpp, screen_flags);
                    draw_gfx (screen);
                    break;
                case SDLK_f:
                    screen_flags ^= SDL_FULLSCREEN;
                    int w, h;
                    info = SDL_GetVideoInfo ();
                    w = info->current_w;
                    h = info->current_h;
                    SDL_Surface *temp = SDL_CreateRGBSurface (screen_flags,
                            width, height, screen_bpp, 0, 0, 0, 0);
                    draw_gfx (temp);
                    screen = SDL_SetVideoMode (width, height,
                            screen_bpp, screen_flags);
                    draw_surface_sdl (temp, screen, 0, 0);
                    width = w;
                    height = h;
                    SDL_ShowCursor (! SDL_ShowCursor (-1)); // toggle
                    SDL_Flip (screen);
                    SDL_FreeSurface (temp);
                    break;
                case SDLK_UP:
                    if (cur_pos >= 0 && playlist [cur_pos - offset] != NULL &&
                            playlist [cur_pos - offset]->next != NULL)
                    status = mpd_run_play_pos (client,
                            playlist [cur_pos - offset]->next->start);
                    break;
                case SDLK_DOWN:
                    if (cur_pos >= 0 && playlist [cur_pos - offset] != NULL &&
                            playlist [cur_pos - offset]->prev != NULL)
                    status = mpd_run_play_pos (client,
                            playlist [cur_pos - offset]->prev->start);
                    break;
                    break;
                case SDLK_LEFT:
                    status = mpd_run_previous (client);
                    break;
                case SDLK_RIGHT:
                    status = mpd_run_next (client);
                    break;
                case SDLK_p:
                    status = mpd_run_toggle_pause (client);
                    break;
                case SDLK_PLUS:
                    status = mpd_run_change_volume (client, 10);
                case SDLK_EQUALS:
                    status = mpd_run_change_volume (client, 10);
                    break;
                case SDLK_KP_PLUS:
                    status = mpd_run_change_volume (client, 10);
                    break;
                case SDLK_MINUS:
                    status = mpd_run_change_volume (client, -10);
                    break;
                case SDLK_KP_MINUS:
                    status = mpd_run_change_volume (client, -10);
                    break;
            }

            if (!status)
                fprintf (stderr, "%s: mpd error: %s\n", prog,
                        mpd_connection_get_error_message (client));
            else if (status == -1)
                break;

        }
        else if (event.type == SDL_VIDEORESIZE)
        {
            w = event.resize.w;
            h = event.resize.h;
        }
    }

    if (w || h)
    {
        SDL_Surface *temp = SDL_CreateRGBSurface (screen_flags, w, h,
                screen_bpp, 0, 0, 0, 0);
        draw_gfx (temp);
        screen = SDL_SetVideoMode (w, h, screen_bpp, screen_flags);
        draw_surface_sdl (temp, screen, 0, 0);
        SDL_Flip (screen);
        SDL_FreeSurface (temp);
    }

    return 0;
}

void finish_sdl (void)
{
    SDL_FreeSurface (screen);
    SDL_Quit ();
}

void wait_sdl (void)
{
    SDL_Delay ((Uint32) 100);
}

int draw_surface_sdl (SDL_Surface *src, SDL_Surface *dst, int x, int y)
{
    // this is a "dumb" drawing method, but it is easier to read and still fast
    if (src == NULL)
        return 1;

    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface (src, NULL, dst, &offset);

    return 0;
}

SDL_Surface *scale_surface_sdl (SDL_Surface *sur, int w, int h)
{
    SDL_Surface *scaled = NULL;
    int flags;

    double dw = (double) w / sur->w;
    double dh = (double) h / sur->h;

    flags = SMOOTHING_ON;

    scaled = rotozoomSurfaceXY (sur, 0, dw, dh, flags);

    if (scaled == NULL)
    {
        fprintf (stderr, "%s: could not scale image\n", prog);
        return NULL;
    }

    return scaled;
}

SDL_Surface *load_img_sdl (char *s, int size)
{
    if (s == NULL)
        return NULL;

    SDL_Surface *img = NULL;
    SDL_Surface *optimized_img = NULL;

    if (size == -1)
        img = IMG_Load (s); // load from file
    else if (size <= 0)
        return NULL;
    else // load from char *
    {
        SDL_RWops *rw = SDL_RWFromMem (s, size);
        if (rw == NULL)
            fprintf (stderr, "rwops failed\n");
        img = IMG_Load_RW (rw, 1);
    }

    if (img == NULL)
    {
        fprintf (stderr, "%s: sdl error: %s\n", prog, SDL_GetError ());
        return NULL;
    }

    optimized_img = SDL_DisplayFormat (img);
    SDL_FreeSurface (img);

    if (optimized_img == NULL)
    {
        fprintf (stderr, "%s: cannot optimize %s: %s\n",
                prog, s, SDL_GetError ());
        return NULL;
    }

    return optimized_img;
}
