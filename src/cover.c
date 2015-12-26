#include <stdio.h>
#include <unistd.h>
#include <string.h> // strlen

#include "cover.h"

#include "options.h" // globals
#include "sdl.h"  // load_img_sdl
#include "id3v2lib/id3v2lib.h"

// internal function prototypes
SDL_Surface *load_embedded (const char *);
SDL_Surface *load_local (const char *);
char *trim_path (char *);

// external functions
SDL_Surface *get_cover (struct mpd_song *song)
{
    if (song == NULL)
        return NULL;
    char *file = get_path_mpd (song, NULL);
    SDL_Surface *sur = NULL;

    if (local)
    {
        sur = load_local (file);
        if (sur == NULL && verbose)
            printf ("%s: no local covers in the same folder as %s\n",
                    prog, file);
        else if (sur != NULL)
        {
            free (file);
            return sur;
        }
    }

    if (embedded)
    {
        sur = load_embedded (file);
        if (sur == NULL && verbose)
            printf ("%s: no embedded covers in%s\n", prog, file);
        else if (sur != NULL)
        {
            free (file);
            return sur;
        }
    }

    free (file);
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
        return NULL;
    }

    ID3v2_frame *frame = tag_get_album_cover (tag);

    if (frame == NULL)
    {
        free_tag (tag);
        return NULL;
    }

    ID3v2_frame_apic_content *album_content = parse_apic_frame_content (frame);

    if (album_content == NULL)
    {
        free_tag  (tag);
        return NULL;
    }

    SDL_Surface *sur =
        load_img_sdl (album_content->data, album_content->picture_size);
    free_tag (tag);
    free_apic_content (album_content);
    return sur;
}

SDL_Surface *load_local (const char *file)
{
    if (file == NULL)
        return NULL;

    char *folder = trim_path ((char *) file);

    if (folder == NULL)
        return NULL;

    SDL_Surface *sur = NULL;
    int len = 12;
    char name [] [11] = {   "cover.png", "front.png", "folder.png",
                            "cover.jpg", "front.jpg", "folder.jpg",
                            "cover.gif", "front.gif", "folder.gif",
                            "cover.bmp", "front.bmp", "folder.bmp", };
    char s [11 + strlen (folder)];
    int i = 0;

    for (i = 0; i < len; i++)
    {
        strcpy (s, folder);
        strcat (s, name [i]);

        if (access (s, F_OK) == 0)
        {
            sur = load_img_sdl (s, -1);
            // REMOVE ME
            printf ("blurring...\n");
            sur = blur_img_sdl (sur);
            printf ("done.\n");
            // REMOVE ME
            break;
        }
    }

    free (folder);
    return sur;
}

char *trim_path (char *file)
{
    int i;
    char *s = malloc (sizeof (char) * ((i = strlen (file)) + 2));
    strcpy (s, file);

    if (s == NULL)
    {
        fprintf (stderr, "%s: out of space\n", prog);
        exit (1);
    }

    while (i > 0 && s [i] != '/')
        s [i--] = '\0';
    if (i <= 0 && s [0] != '/')
    {
        fprintf (stderr, "%s: could not trim path name of %s\n", prog, file);
        free (s);
        return NULL;
    }

    return s;
}
