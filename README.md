# About

upstream is a simple command-line utility for streaming music to a Shoutcast/Icecast server.

# Usage

## Build and install

upstream depends on [libshout](http://www.icecast.org/download.php) and [taglib](http://developer.kde.org/~wheeler/taglib).
Assuming those are installed, build the upstream binary by typing `make`.
To install, copy the upstream binary to your personal `bin` directory.

## Examples

Stream a bunch of MP3 files:

    $ upstream --host example.org *.mp3

Stream OGG files from standard input:

    $ cat *.ogg | upstream --ogg -

# Contributing

This project is available on [GitHub](https://github.com/davidlazar/upstream) and [Bitbucket](https://bitbucket.org/davidlazar/upstream/). You may contribute changes using either.

Please report bugs and feature requests using the [GitHub issue tracker](https://github.com/davidlazar/upstream/issues).
