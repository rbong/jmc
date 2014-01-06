#include <stdio.h>
#include <unistd.h>

#include "cover.h"

#include "options.h" // globals
#include "sdl.h"  // load_img_sdl
#include "id3v2lib/id3v2lib.h"

// internal function prototypes
SDL_Surface *load_embedded (const char *);

// external functions
SDL_Surface *get_cover (struct mpd_song *song)
{
    if (song == NULL)
        return NULL;
    char *path = get_path_mpd (song);
    SDL_Surface *sur = load_embedded (path);
    free (path);
    return sur;
}

// internal functions
SDL_Surface *load_embedded (const char *file)
{
    if (file == NULL)
        return NULL;

    if (access (file, F_OK) == -1)
    {
        fprintf (stderr,"%s: file '%s' does not exist\n", prog, file);
        return NULL;
    }
    if (access (file, R_OK) == -1)
    {
        fprintf (stderr,"%s: file '%s' cannot be read\n", prog, file);
        return NULL;
    }

    ID3v2_tag *tag = load_tag  (file);

    if (tag == NULL)
    {
        fprintf (stderr, "%s: could not load file tags for %s\n", prog, file);
        return NULL;
    }

    ID3v2_frame *frame = tag_get_album_cover (tag);

    if (frame == NULL)
    {
        fprintf (stderr, "%s: could not load file tags for %s\n", prog, file);
        free_tag (tag);
        return NULL;
    }

    ID3v2_frame_apic_content *album_content = parse_apic_frame_content (frame);

    if (album_content == NULL)
    {
        fprintf (stderr, "%s: could not load file tags for %s\n", prog, file);
        free_tag  (tag);
        return NULL;
    }

    SDL_Surface *sur =
        load_img_sdl (album_content->data, album_content->picture_size);
    free_tag (tag);
    free_apic_content (album_content);
    return sur;
}
