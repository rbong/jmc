#include <stdio.h>
#include <string.h> // strlen strcpy strcat strtol strcmp strdup
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mpd/client.h> // mpd_*
#include <SDL/SDL.h> // SDL_*
#include <SDL/SDL_image.h> // SDL_SaveBMP

// resolve conflicting boolean definitions
#undef bool
typedef int bool;
#define true 1
#define false 0

#include "options.h"
#include "gfx.h" // clear_gfx
#include "sdl.h" // finish_sdl
#include "album.h" // is_new_album
#include "mpd.h" // get_path_mpd
#include "cover.h" // get_cover

#define ALL_OPT '\00'
#define FULL_OPT '\01'

// general options
char *music_directory = NULL;
char *dump_directory = NULL;
int bufsize = 15;
bool verbose = false;
// SDL options
int width = 1366;
int height = 768;
int screen_bpp = 32;
int screen_flags = SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF;
bool invert = false;
SDL_Surface *screen = NULL;
// mpd options
struct mpd_connection *client = NULL;
// gfx.c geometry options
int max_size_opt = 300;
bool one_mode = false;
long int bg_color = 0;
double root_size_opt = 0.6;
double size_opt = 0.8;
double pad_opt = 0.04;
double y_off_opt = 0.08;
Uint32 trans;
// cover.c options
bool local = true;
bool embedded = true;
// globals
char *prog = NULL;
char *version = "0.0.14";
int max_path_length = 10000;

// internal funtion prototypes
int get_opt (char *);
int start_sdl (int, int, double);
int set_music_directory (void);
int dump_covers (void);
bool is_int_string_opt (char *, int *);
bool is_hex_string_opt (char *, long int *);
bool is_float_string_opt (char *, double *);
void start_client (char *, unsigned, unsigned);
void print_usage_opt (char);

// add options here
static const char* (option_usage [] [4]) =
{
    //  name    long name       usage
    //  description
    {
        "M",    "host",         "'mpd host'",
        "\tSet the mpd host to connect to. If unset it uses the environment\n"
        "\tvariable MPD_HOST, or the mpd default."
    },
    {
        "P",    "port",         "[mpd port]",
        "\tSet the mpd port to connect with. If unset it uses the\n"
        "\tenvironment variable MPD_PORT, or the mpd default."
    },
    {
        "T",    "timeout",      "[mpd timeout]",
        "\tSet the timeout before the mpd connection expires. If unset it\n"
        "\tuses the environment variable MPD_TIMEOUT, or the mpd default."
    },
    /*
    {
        "R",    "regex",        "{expression}",
        "\tSet a regex expression to filter out unwanted albums. If unset it\n"
        "\tdefaults to empty."
    },
    */
    {
        "b",    "buffer",       "[covers]",
        "\tSet the number of images to load into memory at once. If it is\n"
        "\tsmaller than the number of albums shown on the screen, there will\n"
        "\tbe blank spaces. If it is too large it may take some time to\n"
        "\tcatalog albums, or the program may run out of memory. If unset it\n"
        "\tdefaults to 15. Must be positive or 0."
    },
    {
        "m",    "max",          "[size]",
        "\tSet the maximum size of albums. If unset it defaults to 300. Must\n"
        "\tbe positive."
    },
    {
        "w",    "window",       "[width] [height]",
        "\tSet the size of the window and disallow resizing. If unset it\n"
        "\tdefaults to 800x600 and resizable. Must be positive."
    },
    {
        "d",    "dimensions",   "[width] [height]",
        "\tSet the fullscreen size. It should be set to your exact monitor\n"
        "\tresolution. If unset it defaults to 1366x768. Must be positive."
    },
    {
        "c",    "color",        "[hexidecimal color]",
        "\tSet the background color of the window. If unset it defaults to\n"
        "\tblack (000000)."
    },
    {
        "a",    "album",        "[percent]",
        "\tSet the size of general albums using a decimal percentage of\n"
        "\tcurrently playing album size. If unset it defaults to 0.8. Must\n"
        "\tbe positive or 0."
    },
    {
        "A",    "cur-album",    "[percent]",
        "\tSet the size of the currently playing album using a decimal\n"
        "\tpercentage of screen height. If unset it defaults to 0.6. Must be\n"
        "\tpositive or 0."
    },
    {
        "p",    "padding",      "[percent]",
        "\tSet the size of the padding between album using a decimal\n"
        "\tpercentage of general album size. If unset it defaults to 0.04."
    },
    {
        "t",    "transparency", "[percent]",
        "\tSet the visibility of the reflection using a decimal percentage.\n"
        "\tIf unset it defaults to 0.1. Must be between 0 and 1."
    },
    {
        "y",    "y-off",        "[percent]",
        "\tSet the amount to bump up the currently-playing-album using a\n"
        "\tdecimal percentage of general album size. If unset it defaults to\n"
        "\t0.08."
    },
    {
        "D",    "directory",    "/path/to/music",
        "\tSet the folder that the music database is contained in. If unset,\n"
        "\tit assumes that mpd is configured for a local socket and asks the\n"
        "\tserver for the name of the directory."
    },
    {
        "e",    "no-embed",     "(no parameters)",
        "\tDisable looking for covers inside of ID3 tags. If unset jmc will\n"
        "\tlook for embedded covers."
    },
    {
        "l",    "no-local",     "(no parameters)",
        "\tDisable looking for covers inside of folders songs are found in.\n"
        "\tif unset jmc will look for local covers."
    },
    {
        "1",    "one-mode",     "(no parameters)",
        "\tShow only one album. If unset it defaults to off (classic mode).\n"
    },
    {
        "i",    "invert",       "(no parameters)",
        "\tInvert the next/previous album controls. If unset it defaults to\n"
        "\toff."
    },
    {
        "u",    "dump",         "/path/to/dump",
        "\tGets the cover of every item in the database and dumps it to the\n"
        "\tpath specified. The program then exits."
    },
    {
        "V",    "verbose",      "(no parameters)",
        "\tPrint excess output during usage. If unset it defaults to off."
    },
    {
        "h",    "help",         "(no parameters)",
        "\tPrint the usage of all options and exit."
    },
    {
        "H",    "help-all",     "(no parameters)",
        "\tPrint the usage of all options with description and exit."
    },
    {
        "v",    "version",      "(no parameters)",
        "\tPrint the version of the program and exit."
    },
    { "\0" }
};

// external functions

// add option behaviour here
void parse_opt (char **argv)
{
    char *temp = NULL;
    // default values for mpd/startclient
    char *host = NULL;
    unsigned port = 0;
    unsigned timeout = 0;
    int w = 800, h = 600;
    double reflect = 0.1;

    // temp variables
    int i, j;
    double d;

    // set program name
    prog = *(argv++);

    for ( ; *argv != NULL; argv++)
    {
        char argchar = get_opt (*argv);

        switch (argchar)
        {
        case '\0':
            fprintf (stderr, "%s: unrecognized option %s\n", prog, *argv);
            break;
        case 'M':
            temp = *(++argv);
            if (temp == NULL || temp [0] == '-')
            {
                print_usage_opt ('H');
                --argv;
            }
            else
                host = temp;
            break;
        case 'P':
            temp  = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i) || i < 0)
            {
                print_usage_opt ('P');
                --argv;
            }
            else
                port = i;

            break;
        case 'T':
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i) || i < 0)
            {
                print_usage_opt ('T');
                --argv;
            }
            else
                timeout = i;
            break;
        case 'b':
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i) || i < 0)
            {
                print_usage_opt ('b');
                --argv;
            }
            else
            {
                bufsize = i;
                if (bufsize < 3)
                    bufsize = 3;
                if (i == 1)
                    one_mode = true;
            }
            break;
        case 'm':
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i) || i <= 0)
            {
                print_usage_opt ('m');
                --argv;
            }
            else
                max_size_opt = i;
            break;
        case 'w':
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i) || i <= 0)
            {
                print_usage_opt ('w');
                --argv;
                break;
            }
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &j) || j <= 0)
            {
                print_usage_opt ('w');
                --argv;
            }
            else
            {
                screen_flags &= ~SDL_RESIZABLE;
                w = i;
                h = j;
            }
            break;
        case 'd':
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &i))
            {
                print_usage_opt ('d');
                --argv;
                break;
            }
            temp = *(++argv);
            if (temp == NULL || ! is_int_string_opt (temp, &j) || j <= 0)
            {
                print_usage_opt ('s');
                --argv;
            }
            else
            {
                width = i;
                height = j;
            }
            break;
        case 'c':
            temp = *(++argv);
            if (! is_hex_string_opt (temp, &bg_color))
            {
                print_usage_opt ('c');
                --argv;
            }
            break;
        case 'a':
            temp = *(++argv);
            if (temp == NULL || ! is_float_string_opt (temp, &d) || d < 0)
            {
                print_usage_opt ('a');
                --argv;
            }
            else
                size_opt = d;
            break;
        case 'A':
            temp = *(++argv);
            if (temp == NULL || ! is_float_string_opt (temp, &d) || d < 0)
            {
                print_usage_opt ('A');
                --argv;
            }
            else
                root_size_opt = d;
            break;
        case 'p':
            temp = *(++argv);
            if (temp == NULL || ! is_float_string_opt (temp, &pad_opt))
            {
                print_usage_opt ('p');
                --argv;
            }
            break;
        case 'y':
            temp = *(++argv);
            if (temp == NULL || ! is_float_string_opt (temp, &y_off_opt))
            {
                print_usage_opt ('y');
                --argv;
            }
            break;
        case 'V':
            verbose = true;
            break;
        case 'l':
            local = false;
            break;
        case 'e':
            embedded = false;
            break;
        case '1':
            one_mode = true;
            break;
        case 'i':
            invert = true;
            break;
        case 't':
            temp = *(++argv);
            if (! is_float_string_opt (temp, &d) || d > 1 || d < 0)
            {
                print_usage_opt ('t');
                --argv;
            }
            else
                reflect = d;
            break;
        case 'D':
            music_directory = *(++argv);
            if (music_directory == NULL || music_directory [0] == '-')
            {
                print_usage_opt ('D');
                --argv;
                music_directory = NULL;
            }
            else if
                (music_directory [(i = strlen (music_directory) - 1)] != '/')
            {
                char *s = malloc (sizeof (char) * (i + 2));
                if (s == NULL)
                {
                    fprintf (stderr, "%s: out of space\n", prog);
                    exit (1);
                }
                strcpy (s, music_directory);
                s [++i] = '/';
                s [++i] = '\0';
                music_directory = s;
            }
            break;
        case 'u':
            dump_directory = *(++argv);
            if (dump_directory == NULL || dump_directory [0] == '-')
            {
                print_usage_opt ('u');
                --argv;
                dump_directory = NULL;
            }
            else if
                (dump_directory [(i = strlen (dump_directory) - 1)] != '/')
            {
                char *s = malloc (sizeof (char) * (i + 2));
                if (s == NULL)
                {
                    fprintf (stderr, "%s: out of space\n", prog);
                    exit (1);
                }
                strcpy (s, dump_directory);
                s [++i] = '/';
                s [++i] = '\0';
                dump_directory = s;
            }
            break;
        case 'h':
            print_usage_opt (ALL_OPT);
            exit (0);
            break;
        case 'H':
            print_usage_opt (FULL_OPT);
            exit (0);
            break;
        case 'v':
            printf ("jmc %s\n", version);
            exit (0);
            break;
        default:
            fprintf (stderr, "%s: unrecognized option %s\n", prog, *argv);
            break;
        }
    }

    if (! local && ! embedded)
        fprintf (stderr, "%s: warning:"
                "disabling all cover grabbing methods\n", prog);


    start_client (host, port, timeout);
    // post-arg mpd settings
    if (music_directory == NULL)
        set_music_directory ();

    start_sdl (w, h, reflect);

    if (dump_directory != NULL)
    {
        exit (dump_covers ());
    }
}

// internal functions

bool is_int_string_opt (char *s, int *i)
{
    char *t = s;
    if (*s == '-')
        s++;
    while (*s != '\0' && isdigit (*s))
        s++;
    if (*s == '\0')
    {
        *i = atoi (t);
        return true;
    }
    return false;
}

bool is_hex_string_opt (char *s, long int *l)
{
    if (strlen (s) != 6)
        return false;
    char *t = s;
    char c;
    while (*s != '\0' &&
            (isdigit (*s) || ((c = toupper (*s)) <= 'F' && c >= 'A')))
        s++;
    if (*s == '\0')
    {
        *l = strtol ((const char *) t, NULL, 16);
        return true;
    }
    return false;
}

bool is_float_string_opt (char *s, double *d)
{
    bool decimal = false;
    char *t = s;
    if (*s == '-')
        s++;
    while (*s != '\0' && (isdigit (*s) || *s == '.'))
    {
        if (*s == '.')
        {
            if (decimal)
                break;
            else
                decimal = true;
        }
        s++;
    }
    if (*s == '\0')
    {
        *d = atof ((const char *) t);
        return true;
    }
    return false;
}

void print_usage_opt (char o)
{
    printf ("usage:\n");
    char c;

    for (int i = 0; (c = option_usage [i] [0] [0]) != '\0'; i++)
        if (c == o || o == ALL_OPT || o == FULL_OPT)
        {
            printf ("-%-4c --%-16s%s\n", c, option_usage [i] [1],
                    option_usage [i] [2]);
            if (o == FULL_OPT)
                printf ("%s\n", option_usage [i] [3]);
            else if (o != ALL_OPT)
                break;
        }
}

int get_opt (char *s)
{
    int len = strlen (s);

    if (len < 2)
        return '\0';

    if (s [0] != '-')
        return '\0';

    if (len == 2)
        return s [1];

    if (s [1] != '-')
        return '\0';

    s += 2;
    int c;
    for (int i = 0; (c = option_usage [i] [0] [0]) != '\0'; i++)
    {
        if (strcmp (s, option_usage [i] [1]) == 0)
            break;
    }
    return c;
}

int start_sdl (int w, int h, double reflect)
{
    if (SDL_Init (SDL_INIT_EVERYTHING) == -1)
    {
        fprintf (stderr, "%s: %s\n", prog, SDL_GetError ());
        return -1;
    }

    screen = SDL_SetVideoMode (w, h, screen_bpp, screen_flags);
    // fix automatically set tiled dimensions
    const SDL_VideoInfo *info = SDL_GetVideoInfo ();
    screen = SDL_SetVideoMode (info->current_w, info->current_h,
                               screen_bpp, screen_flags);
    if (screen == NULL)
    {
        fprintf (stderr, "%s: %s\n", prog, SDL_GetError ());
        SDL_Quit ();
        return -1;
    }

    SDL_WM_SetCaption (prog, NULL);

    clear_gfx (screen);

    if (SDL_Flip (screen) == -1)
    {
        fprintf (stderr, "%s: %s\n", prog, SDL_GetError ());
        finish_sdl ();
        return -1;
    }

    trans = 0xFF * (1 - reflect);
    trans = (trans << 16) | (trans << 8) | trans;

    return 0;
}

void start_client (char *host, unsigned port, unsigned timeout)
{
    // default values found in options.c
    client = mpd_connection_new (host, port, timeout);

    if (mpd_connection_get_error (client) != MPD_ERROR_SUCCESS) 
    {
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        exit (-1);
    }
}

int set_music_directory (void)
{
    if (mpd_connection_cmp_server_version (client, 0, 17, 0) < 0)
    {
        fprintf (stderr, "%s: please update mpd to at least 0.17.1\n", prog);
        return 1;
    }

    if (! mpd_send_command (client, "config", NULL))
    {
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        return 1;
    }

    struct mpd_pair *pair =
        mpd_recv_pair_named (client, "music_directory");

    if (pair != NULL)
    {
        int length;
        music_directory = strdup (pair->value);
        char *temp = realloc
            (music_directory, sizeof (char) * (strlen (music_directory) + 2));
        if (temp != NULL)
            music_directory = temp;
        // should exit here
        mpd_return_pair (client, pair);

        if (music_directory [(length = strlen (music_directory))] != '/')
        {
            strcat (music_directory, "/");
        }
    }
    else
    {
        (void) mpd_connection_get_error (client);
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        return 1;
    }

    if (!  (mpd_response_finish (client)
        || mpd_connection_clear_error (client)))
    {
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        return 1;
    }

    return 0;
}

int dump_covers (void)
{
    struct mpd_entity *entity = NULL;
    char *last = NULL;

    if (! mpd_send_list_all (client, NULL))
    {
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        return 1;
    }

    while ((entity = mpd_recv_entity (client)) != NULL)
    {
        struct mpd_song *song = NULL;
        SDL_Surface* cover = NULL;
        char* file = NULL;

        if (mpd_entity_get_type (entity) != MPD_ENTITY_TYPE_SONG)
        {
            continue;
        }

        song = (struct mpd_song *) mpd_entity_get_song (entity);
        if (song == NULL)
        {
            fprintf (stderr, "%s: %s\n", prog,
                    mpd_connection_get_error_message (client));
            continue;
        }

        file = get_path_mpd (song, dump_directory);
        if (file == NULL)
            continue;

        // convert filename to folder name
        int i;
        for (i = strlen (file) - 1; i > 0 && file [i] != '/'; i--);
        if (file [i] == '/')
            file [i + 1] = '\0';
        else
        {
            fprintf (stderr, "%s: not a valid pathname: %s\n", prog, file);
            free (file);
            continue;
        }
        if (last != NULL && strcmp (file, last) == 0)
        {
            free (file);
            continue;
        }

        printf ("folder: %s\n", file);

        cover = get_cover (song);
        if (cover == NULL)
        {
            free (file);
            continue;
        }

        // create folder if it does not exist
        struct stat buf = { 0 };
        if (stat (file, &buf) < 0)
        {
            char *cmd = strdup ("mkdir -p ");
            char *temp = realloc (cmd, sizeof (char) *
                    (strlen (cmd) + strlen (file) + 3));
            if (temp != NULL)
                cmd = temp;
            else
                continue;
            strcat (cmd, "\"");
            strcat (cmd, file);
            strcat (cmd, "\"");
            system (cmd);
            free (cmd);
        }
        char *folder = strdup (file);

        // add cover name to folder name
        char *temp = realloc (file, sizeof (char) * (strlen (file) + 10));
        if (temp != NULL)
            file = temp;
        else
        {
            free (folder);
            free (file);
            continue;
        }
        strcat (file, "cover.bmp");

        // if (access (fname, F_OK) != -1)
        // {
        //     // file exists
        // }


        if (SDL_SaveBMP (cover, file) < 0)
        {
            free (folder);
            fprintf (stderr, "%s: failure saving %s: %s\n",
                    prog, file, SDL_GetError ());
        }
        else
        {
            free (last);
            last = folder;
        }

        // free (song);
        free (file);
        SDL_FreeSurface (cover);
    }
    if (mpd_connection_get_error (client) != MPD_ERROR_SUCCESS)
    {
        fprintf (stderr, "%s: %s\n", prog,
                mpd_connection_get_error_message (client));
        return 1;
    }

    return 0;
}
