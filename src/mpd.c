#include <mpd/client.h>

#include <stdio.h>
#include <string.h>

#include "options.h" // globals
#include "mpd.h"
#include "playlist.h" // update_playlist playlist object
#include "gfx.h" // clear_gfx update_gfx

int is_same_tag_mpd
    (struct mpd_song *s1, struct mpd_song *s2, enum mpd_tag_type tag)
{
    char *t1 = (char *) mpd_song_get_tag (s1, tag, 0);
    char *t2 = (char *) mpd_song_get_tag (s2, tag, 0);

    if (t1 == NULL || t2 == NULL)
        return -1;
    if (strcmp (t1, t2) == 0)
        return 0;
    return 1;
}

char *get_path_mpd (struct mpd_song *song)
{
    if (song == NULL)
    {
        fprintf (stderr, "%s: could not get path (song is NULL)\n", prog);
        return NULL;
    }

    const char *s = mpd_song_get_uri (song);
    char *path = malloc (sizeof (char) *
            (strlen (s) + strlen (music_directory) + 1));
    strcpy (path, music_directory);
    strcat (path, s);

    return path;
}

int poll_mpd (void)
{
    struct mpd_status *status = NULL;

    mpd_send_status (client);
    if ((status = mpd_recv_status (client)) == NULL)
    {
        fprintf (stderr, "%s: mpd error: %s\n", prog,
                mpd_connection_get_error_message (client));
        return -1;
    }

    int new_ver;
    int new_pos;

    new_ver = mpd_status_get_queue_version (status);
    new_pos = mpd_status_get_song_pos (status);

    mpd_status_free (status);

    // out of bounds, update first
    if (new_pos != -1 &&
            (cur_ver == 0 ||
             new_ver != cur_ver ||
             new_pos > (play_len + offset) ||
             new_pos < offset))
    {
        cur_pos = new_pos;
        update_playlist (new_ver);
        draw_gfx (screen);
        SDL_Flip (screen);
        cur_ver = new_ver;
    }
    // within bounds, draw first
    else if (new_pos - offset >= 0 &&
            playlist [cur_pos - offset] != playlist [new_pos - offset])
    {
        cur_pos = new_pos;
        draw_gfx (screen);
        SDL_Flip (screen);
        if ((next_is_nul && ! is_last_song) ||
                (prev_is_nul && ! is_first_song))
        {
            update_playlist (new_ver);
            draw_gfx (screen);
            SDL_Flip (screen);
        }
    }
    // empty playlist or not playing, clear the screen
    else if (new_pos == -1 && cur_pos != -1)
    {
        clear_gfx (screen);
        SDL_Flip (screen);
        cur_pos = -1;
    }
    return 0;
}
