#include <stdio.h>

#include "options.h" // parse_opt
#include "mpd.h" // poll_mpd
#include "sdl.h" // finish_sdl poll_sdl
#include "playlist.h" // free_playlist

int loop ()
{
    while (true)
    {
        int exit_status;

        if ((exit_status = poll_mpd ()) == -2)
        {
            free_playlist ();
            finish_sdl ();
            exit (exit_status);
        }

        if ((exit_status = poll_sdl ()) != 0)
        {
            free_playlist ();
            finish_sdl ();
            exit (exit_status);
        }

        wait_sdl ();
    }
}

int main (int argc, char **argv)
{
    parse_opt (argv);

    loop ();

    fprintf (stderr, "%s: unknown fatal error\n", prog);
    exit (-5);; // impossible
}
