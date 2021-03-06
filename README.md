jmc 0.0.11
===

![fullscreen-mode](https://raw.github.com/rbong/jmc/master/img/screen1.png)

jmc is a client for the mpd music server software. It connect to the server and
fetches the cover art of albums in the playlist, and then it displays them in a
pleasing manner based on command line options. Full client capabilities are
planned.

dependencies
===
jmc requires libmpdclient, SDL, SDL\_image, and SDL\_gfx

installation
===
Once you have the dependencies, type
```
make
sudo make install
```
if you are on osx, try
```
make osx
sudo make install
```
This is not guaranteed to work.

mpd configuration
===
At the moment, jmc only supports local servers. You should set this option or
similar in your mpd.conf:
```
bind_to_address		"~/.mpd/socket"
```
If you would prefer to use a network server rather than a socket, jmc will
still work if you set the -D option with the path to your music database.
You can set the port, host, and timeout manually from jmc, but setting the
MPD\_HOST, MPD\_PORT, and MPD\_TIMEOUT environment variables is the standard way
to ensure that these are set automatically.

cover configuration
===
Currently jmc supports embedded id3 covers in mp3 files and covers in the same
folder as songs.

planned features
===
- Automatic cover fetching (with MusicBrainz and Amazon support)
- Network server support
- Cover databases
- Optional progress bar and album info display
- Basic controls (song changing, album changing, view modification etc.)
- Quicker, seamless buffering
