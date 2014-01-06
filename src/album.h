#ifdef ALBUM
album *new_album (album *, album *, struct mpd_song *);
int is_new_album (struct mpd_song *, struct mpd_song *);
#endif

#ifndef ALBUM
#define ALBUM
typedef struct album
{
    struct album *prev;
    struct album *next;
    int len;
    SDL_Surface *cover;
} album;

#include "album.c"
#endif
