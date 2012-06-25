/* upstream - stream files to a ShoutCast/Icecast server.
 * Author: David Lazar
 * License: MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <shout/shout.h>
#include <taglib/tag_c.h>

#define BUFSIZE 4096
#define VERSION "0.1.0"

static char *program_name;
static char *host = "localhost";
static int port = 8000;
static char *user = "source";
static char *password = "hackme";
static char *mountpoint = "/live";
static int format = SHOUT_FORMAT_MP3;
static int protocol = SHOUT_PROTOCOL_HTTP;

static long unsigned int total_bytes = 0;
static shout_t *shout;

static const char *format_str[] = {
    [SHOUT_FORMAT_OGG] = "ogg",
    [SHOUT_FORMAT_MP3] = "mp3",
    [SHOUT_FORMAT_WEBM] = "webm"
};

static const char *protocol_str[] = {
    [SHOUT_PROTOCOL_HTTP] = "http",
    [SHOUT_PROTOCOL_XAUDIOCAST] = "audiocast",
    [SHOUT_PROTOCOL_ICY] = "icy"
};

static struct option long_options[] = {
    {"host",  required_argument, 0, 'h'},
    {"port",  required_argument, 0, 'p'},
    {"user", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'k'},
    {"mount", required_argument, 0, 'm'},
    {"mp3", no_argument, 0, 'l'},
    {"ogg", no_argument, 0, 'o'},
    {"http", no_argument, 0, 't'},
    {"audiocast", no_argument, 0, 'a'},
    {"icy", no_argument, 0, 'i'},
    {"help", no_argument, 0, 'q'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

void usage() {
    printf("Usage: %s [OPTION]... [FILE]...\n"
    "Stream FILE(s) to a Shoutcast/Icecast server.\n"
    "\n"
    "Connection settings:\n"
    "\n"
    "  --host=HOST          Connect to HOST        {%s}\n"
    "  --port=PORT          Connect to PORT        {%d}\n"
    "  --user=USER          Connect as USER        {%s}\n"
    "  --password=PASSWORD  Connect with PASSWORD  {%s}\n"
    "\n"
    "Stream settings:\n"
    "\n"
    "  --mount=FILE         Set stream mountpoint  {%s}\n"
    "  --mp3, --ogg         Set stream format      {%s}\n"
    "  --http, --icy,       Set stream protocol    {%s}\n"
    "  --audiocast\n"
    "\n"
    "Misc. options:\n"
    "\n"
    "  --help               Print this help message\n"
    "  --version            Print version info\n"
    "\n"
    "When FILE is - standard input is read.\n"
    "Values in {} indicate option defaults.\n"
    "Issues: github.com/davidlazar/upstream\n",
    program_name, host, port, user, password, mountpoint,
    format_str[format], protocol_str[protocol]);
}

static inline void try(int ret, char *msg) {
    if (ret != SHOUTERR_SUCCESS) {
        printf("%s: %s\n", msg, shout_get_error(shout));
        exit(1);
    }
}

void stream(FILE *fp) {
    unsigned char buf[BUFSIZE];
    size_t read;

    while (1) {
        read = fread(buf, 1, BUFSIZE, fp);
        total_bytes += read;

        if (read > 0) {
            if (shout_send(shout, buf, read) != SHOUTERR_SUCCESS) {
                fprintf(stderr, "Send error: %s\n", shout_get_error(shout));
                break;
            }
        } else {
            break;
        }

        shout_sync(shout);
    }
}

int handle_metadata(char *file, char *metadata) {
    TagLib_File *fileref;
    TagLib_Tag *tag;

    fileref = taglib_file_new(file);
    if (fileref == NULL) {
        return 1;
    }

    tag = taglib_file_tag(fileref);
    if (tag == NULL) {
        return 1;
    }

    snprintf(metadata, BUFSIZE, "%s - %s", taglib_tag_artist(tag), taglib_tag_title(tag));

    if (format != SHOUT_FORMAT_MP3) {
        return 0;
    }

    // metadata only supported for mp3 streams
    shout_metadata_t *shout_metadata = shout_metadata_new();
    try(shout_metadata_add(shout_metadata, "song", metadata), "Error adding metadata");
    try(shout_set_metadata(shout, shout_metadata), "Error setting metadata");
    shout_metadata_free(shout_metadata);

    return 0;
}

int main(int argc, char **argv) {
    int index, c;

    program_name = argv[0];

    int option_index = 0;
    while (1) {
        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'u':
            user = optarg;
            break;
        case 'k':
            password = optarg;
            break;
        case 'l':
            format = SHOUT_FORMAT_MP3;
            break;
        case 'o':
            format = SHOUT_FORMAT_OGG;
            break;
        case 't':
            protocol = SHOUT_PROTOCOL_HTTP;
            break;
        case 'a':
            protocol = SHOUT_PROTOCOL_XAUDIOCAST;
            break;
        case 'i':
            protocol = SHOUT_PROTOCOL_ICY;
            break;
        case 'q':
            usage();
            return 0;
        case 'v':
            printf("%s\n", VERSION);
            return 0;
        case '?':
            usage();
            return 1;
        default:
            fprintf(stderr, "getopt unexpectedly returned %d (%c)", c, c);
            return 1;
        }
    }

    shout_init();

    if (!(shout = shout_new())) {
        printf("Could not allocate shout_t\n");
        return 1;
    }

    try(shout_set_host(shout, host), "Error setting hostname");
    try(shout_set_port(shout, port), "Error setting port");
    try(shout_set_user(shout, user), "Error setting user");
    try(shout_set_password(shout, password), "Error setting password");
    try(shout_set_mount(shout, mountpoint), "Error setting mountpoint");
    try(shout_set_format(shout, format), "Error setting format");
    try(shout_set_protocol(shout, protocol), "Error setting protocol");

    fprintf(stderr, "Server: %s@%s:%d\n", shout_get_user(shout),
      shout_get_host(shout), shout_get_port(shout));
    fprintf(stderr, "Mountpoint: %s\n", shout_get_mount(shout));

    try(shout_open(shout), "Error opening connection");
    fprintf(stderr, "Connected.\n\n");

    FILE *fp;
    char *file;
    char metadata[BUFSIZE];
    // iterate over non-option arguments (files to stream)
    for (index = optind; index < argc; index++) {
        file = argv[index];
        if (strcmp(file, "-") == 0) {
            fp = stdin;
            freopen(NULL, "rb", stdin);
        } else if ((fp = fopen(file, "rb")) == NULL) {
            printf("cannot open %s\n", file);
            return 1;
        }

        metadata[0] = 0;
        if (handle_metadata(file, metadata) == 0) {
            fprintf(stderr, "Streaming: %s (%s)\n", file, metadata);
        } else {
            fprintf(stderr, "Streaming: %s\n", file);
        }
        stream(fp);
    }

    fprintf(stderr, "Total bytes read: %lu\n", total_bytes);

    shout_close(shout);
    shout_shutdown();

    return 0;
}
