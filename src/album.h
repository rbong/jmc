#ifndef ALBUM
#define ALBUM

typedef struct album
{
    struct album *prev;
    struct album *next;
    int start;
    int end;
    SDL_Surface *cover;
} album;

album *new_album (album *, album *, struct mpd_song *, int, int);
int is_new_album (struct mpd_song *, struct mpd_song *);

#endif

