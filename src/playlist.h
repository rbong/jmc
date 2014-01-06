#ifdef PLAYLIST
#include "album.h"
extern int offset;
extern int cur_pos;
extern int play_len;
extern unsigned cur_ver;
extern bool is_last_song;
extern bool is_first_song;
extern album **playlist;

int update_playlist (int);
void free_playlist (void);
#endif

#ifndef PLAYLIST
#define PLAYLIST
#include "playlist.c"
#endif
