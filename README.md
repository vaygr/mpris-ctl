# mpris-ctl 
[![MIT Licensed](https://img.shields.io/github/license/mariusor/mpris-ctl.svg)](https://raw.githubusercontent.com/mariusor/mpris-ctl/master/LICENSE)
[![Build status](https://builds.sr.ht/~mariusor/mpris-ctl.svg)](https://builds.sr.ht/~mariusor/mpris-ctl)
[![AUR package](https://img.shields.io/aur/version/mpris-ctl.svg)](https://aur.archlinux.org/packages/mpris-ctl/)
[![Coverity Scan status](https://img.shields.io/coverity/scan/12309.svg)](https://scan.coverity.com/projects/12309)
[![Latest build](https://img.shields.io/github/release/mariusor/mpris-ctl.svg)](https://github.com/mariusor/mpris-ctl/releases/latest)


Is a minimalistic cli tool for controlling audio players exposing a MPRIS DBus interface, targeted at keyboard based WMs.

Its only build/run dependency is on the [C dbus library](https://dbus.freedesktop.org/doc/api/html/index.html).

## Build

To build from source just clone the repository and run make. 
By default the binary is installed in **/usr/local/bin**, but you can provide your own DESTDIR and INSTALL_PREFIX.

```sh
$ git clone https://github.com/mariusor/mpris-ctl.git
$ cd mpris-ctl
$ make 
# make install
```

## Usage

An example of configuration for i3/sway:

```
bindsym XF86AudioPlay exec "mpris-ctl --player active --player inactive pp"
bindsym XF86AudioStop exec "mpris-ctl --player active stop"
bindsym XF86AudioNext exec "mpris-ctl --player active next"
bindsym XF86AudioPrev exec "mpris-ctl --player active prev"
```

The `--player` flag supports passing multiple player names, or the values `active` or `inactive`. 
The active players are considered to be the ones which have the `play_status` be `Playing`, 
and the inactive ones are the ones with the `play_status` `Stopped` or `Paused`.

Eg:

```
mpris-ctl --player Rhythmbox play
mpris-ctl --player active pp
mpris-ctl --player inactive play
```

A more advanced example could be (this requires a notify daemon to be running):

```
set $mpris_notify notify-send "$(mpris-ctl info "%play_status")" \
    "$(mpris-ctl info "%artist_name: %track_name\nOn album '%album_name'")"
bindsym $mod+XF86AudioPlay exec $mpris_notify
# or even:
bindsym XF86AudioPlay exec mpris-ctl pp && $mpris_notify
```

Supported format specifiers for `mpris-ctl info` command:

```
Format specifiers:
    %player_name     prints the player name
    %track_name      prints the track name
    %track_number    prints the track number
    %track_length    prints the track length (seconds)
    %artist_name     prints the artist name
    %album_name      prints the album name
    %album_artist    prints the album artist
    %play_status     prints the playback status
    %shuffle         prints the shuffle mode
    %volume          prints the volume
    %loop_status     prints the loop status
    %position        prints the song position (seconds)
    %bitrate         prints the track's bitrate
    %comment         prints the track's comment
    %full            prints all available information

```

Example: 

```
$ mpris-ctl info "%track_name"
Song 42
$ mpris-ctl info
Song 42 - Bloor - The Best of Bloor

```

## Resources

For discussions related to the project without requiring a Github account please see our mailing list: [https://lists.sr.ht/~mariusor/mpris-tools](https://lists.sr.ht/~mariusor/mpris-tools).

