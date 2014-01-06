jmc
===

![fullscreen-screenshot](https://raw.github.com/rbong/jmc/master/img/screen1.png)

just mpd covers. Type jmc -h or jmc -H for options. mpd must be playing a song
for it to display graphics. jmc can be set into fullscreen mode by pressing f.
Screen dimensions must be set via the -d option. Press q to quit.

dependencies
===
jmc requires libmpdclient, SDL, SDL\_image, and SDL\_gfx

installation
===
Once you have the dependencies, type
make
sudo make install

mpd configuration
===
At the moment, jmc only supports local servers. You should set this option or
similar in your mpd.conf:
bind\_to\_address		"~/.mpd/socket"
If you would prefer to use a network server rather than a socket, jmc will
still work if you set the -D option with the path to your music database.
You can set the port, host, and timeout manually from jmc, but setting the
MPD\_HOST, MPD\_PORT, and MPD\_TIMEOUT environment variables is the standard way
to ensure that these are set automatically.

cover configuration
===
jmc is fairly early in development. Currently jmc only supports embedded id3
covers in mp3 files. Automatic cover fetching is planned, as is support for
unembedded local covers. Once automatic cover fetching is implemented, network
servers will work similar to sockets.
