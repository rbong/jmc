#include <mpd/client.h> // mpd_song, MPD_*
#include <SDL/SDL.h> // SDL_Surface

#include "mpd.h" // is_same_tag_mpd
#include "cover.h" // get_cover

album *new_album (album *prev, album *next, struct mpd_song *song)
{
    album *new = (album *) malloc (sizeof (album));
    new->prev = prev;
    new->next = next;
    new->len = 1;
    new->cover = get_cover (song);
    if (prev != NULL)
        prev->next = new;
    if (next != NULL)
        next->prev = new;
    return new;
}

int is_new_album (struct mpd_song *s1, struct mpd_song *s2)
{
    if (s1 == NULL || s2 == NULL)
        return 0;

    int status;
    /* not a good idea yet
    if ((status = is_same_tag_mpd (s2, s2, MPD_TAG_MUSICBRAINZ_ALBUMID)) != -1)
        return status;
    */
    if ((status = is_same_tag_mpd (s1, s2, MPD_TAG_ALBUM)) != 0)
        return status;
    // check for album artist tag before artist tag
    if ((status = is_same_tag_mpd (s1, s2, MPD_TAG_ALBUM_ARTIST)) != -1)
        return status;
    if ((status = is_same_tag_mpd (s1, s2, MPD_TAG_ARTIST)) != -1)
        return status;
    // artist is untagged- assume it's the same album
    return 0;
}
