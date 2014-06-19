# Upstream

upstream is a simple command-line utility for streaming audio to a
Shoutcast/Icecast server.

# Usage

## Build and install

upstream depends on [libshout](http://www.icecast.org/download.php) and
[taglib](http://developer.kde.org/~wheeler/taglib).
Assuming those are installed, build the upstream binary by typing `make`.
To install, copy the upstream binary to your personal `bin` directory.

## Examples

Stream a bunch of MP3 files:

    $ upstream --host example.org *.mp3

Stream OGG files from standard input:

    $ cat *.ogg | upstream --ogg -
