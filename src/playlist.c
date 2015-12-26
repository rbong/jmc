#include <stdio.h>
#include <mpd/client.h> // mpd_*, MPD_*
#include <SDL/SDL.h> // SDL_FreeSurface

#include "options.h" // globals
#include "playlist.h"
#include "mpd.h" // client variable
#include "album.h" // is_new_album, new_album, album structure

int offset = 0;
int play_len = 0;
int cur_pos = -3;
unsigned cur_ver = 0;
bool is_last_song = false;
bool is_first_song = false;

album **playlist = NULL;

void index_playlist (album *, int);

void free_playlist (void)
{
    if (playlist == NULL)
        return;
    album *cur;

    cur = playlist [0];
    while (cur != NULL)
    {
        album *temp = cur;
        cur = (cur->next);
        if (temp->cover != NULL)
            SDL_FreeSurface (temp->cover);
        free (temp);
    }
    if (playlist != NULL)
        free (playlist);
}

int update_playlist (int new_ver)
{
    offset = cur_pos;

    if (cur_pos >= -1 && cur_ver != 0)
        free_playlist ();
    if (cur_pos < 0)
        return 1;

    struct mpd_song *root_s = mpd_run_get_queue_song_pos (client, offset);
    struct mpd_song *cur_s = NULL;
    struct mpd_song *temp_s = root_s;

    if (verbose)
        printf ("adding album %s\n",
                mpd_song_get_tag (root_s, MPD_TAG_ALBUM, 0));

    album *root_a = new_album (NULL, NULL, temp_s, offset, offset);
    album *cur_a = root_a;
    album *temp_a = root_a;
    int song_count = 1;
    int album_count = 1;

    is_first_song = false;

    // prepend albums
    while (offset > 0 && album_count <= (bufsize/2))
    {
        offset--;
        cur_s = mpd_run_get_queue_song_pos (client, offset);
        if (cur_s == NULL)
            break;
        if (is_new_album (cur_s, temp_s))
        {
            cur_a = new_album (NULL, temp_a, cur_s, offset, offset);
            if (verbose)
                printf ("adding album %s\n",
                        mpd_song_get_tag (cur_s, MPD_TAG_ALBUM, 0));
            temp_a = cur_a;
            album_count++;
        }
        else
            temp_a->start -= 1;
        song_count++;
        if (temp_s != root_s)
            mpd_song_free (temp_s);
        temp_s = cur_s;
    }

    if (cur_s == NULL)
        is_first_song = true;

    album *first = cur_a;
    temp_a = root_a;
    temp_s = root_s;
    int index = cur_pos;

    is_last_song = false;

    // append albums
    while ((cur_s = mpd_run_get_queue_song_pos (client, ++index)) != NULL  &&
            album_count < bufsize)
    {
        if (is_new_album (cur_s, temp_s))
        {
            cur_a = new_album (temp_a, NULL, cur_s, index, index);
            if (verbose)
                printf ("adding album %s\n",
                        mpd_song_get_tag (cur_s, MPD_TAG_ALBUM, 0));
            temp_a = cur_a;
            album_count++;
        }
        else
            temp_a->end += 1;
        song_count++;
        mpd_song_free (temp_s);
        temp_s = cur_s;
    }

    if (cur_s == NULL)
        is_last_song = true;

    mpd_song_free (temp_s);

    index_playlist (first, song_count);

    if (!mpd_response_finish (client))
    {
        fprintf (stderr, "%s: mpd command failed\n", prog);
        return -1;
    }

    if (verbose)
        printf ("playlist updated\n");

    return 0;
}

// internal functions
void index_playlist (album *a, int song_count)
{
    playlist = (album **) malloc (sizeof (album *) * (song_count+1));
    if (playlist == NULL)
    {
        fprintf (stderr, "%s: fatal error, out of space\n", prog);
        exit (-1);
    }

    int index = 0;

    while (a != NULL)
    {
        int len = a->end - a->start;
        for (int i = 0; i <= len; i++)
            playlist [index++] = a;
        a = a->next;
    }

    playlist [index] = NULL;
    play_len = index;
}
