#ifdef MPD
int is_same_tag_mpd (struct mpd_song *, struct mpd_song *, enum mpd_tag_type);
char *get_path_mpd (struct mpd_song *);
int poll_mpd (void);
#endif

#ifndef MPD
#define MPD
#include "mpd.c"
#endif
