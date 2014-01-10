#ifndef OPTIONS
#define OPTIONS

#include <SDL/SDL.h>
#include <stdbool.h>

// SDL options
extern int width;
extern int height;
extern int screen_bpp;
extern int screen_flags;
extern SDL_Surface *screen;

// mpd options
extern struct mpd_connection *client;

// general options
extern bool verbose;
extern int bufsize;

// gfx.c options
extern bool one_mode;
extern int max_size_opt;
extern long int bg_color;
extern double root_size_opt;
extern double size_opt;
extern double pad_opt;
extern double y_off_opt;
extern Uint32 trans;

// cover.c options
extern bool local;
extern bool embedded;

// globals
extern char *prog;
extern char *version;
extern char *music_directory;

// external functions
void parse_opt (char **);

#endif

