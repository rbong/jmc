#ifndef MPD
#define MPD

#include <mpd/client.h>

int is_same_tag_mpd (struct mpd_song *, struct mpd_song *, enum mpd_tag_type);
char *get_path_mpd (struct mpd_song *, char *);
int poll_mpd (void);

#endif

