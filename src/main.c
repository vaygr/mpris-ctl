/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */

#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sstring.h"
#include "sdbus.h"

#define CMD_HELP        "help"
#define CMD_PLAY        "play"
#define CMD_PAUSE       "pause"
#define CMD_STOP        "stop"
#define CMD_NEXT        "next"
#define CMD_PREVIOUS    "prev"
#define CMD_PLAY_PAUSE  "pp"
#define CMD_STATUS      "status"
#define CMD_SEEK        "seek"

#define CMD_LIST        "list"
#define CMD_INFO        "info"
#define ARG_PLAYER      "--player"

#define INFO_DEFAULT_STATUS "%track_name - %album_name - %artist_name"
#define INFO_FULL_STATUS    "Player name:\t" INFO_PLAYER_NAME "\n" \
"Play status:\t" INFO_PLAYBACK_STATUS "\n" \
"Track:\t\t" INFO_TRACK_NAME "\n" \
"Artist:\t\t" INFO_ARTIST_NAME "\n" \
"Album:\t\t" INFO_ALBUM_NAME "\n" \
"Album Artist:\t" INFO_ALBUM_ARTIST "\n" \
"Art URL:\t" INFO_ART_URL "\n" \
"Track:\t\t" INFO_TRACK_NUMBER "\n" \
"Length:\t\t" INFO_TRACK_LENGTH "\n" \
"Volume:\t\t" INFO_VOLUME "\n" \
"Loop status:\t" INFO_LOOP_STATUS "\n" \
"Shuffle:\t" INFO_SHUFFLE_MODE "\n" \
"Position:\t" INFO_POSITION "\n" \
"Bitrate:\t" INFO_BITRATE "\n" \
"Comment:\t" INFO_COMMENT \
""

#define INFO_PLAYER_NAME     "%player_name"
#define INFO_TRACK_NAME      "%track_name"
#define INFO_TRACK_NUMBER    "%track_number"
#define INFO_TRACK_LENGTH    "%track_length"
#define INFO_ARTIST_NAME     "%artist_name"
#define INFO_ALBUM_NAME      "%album_name"
#define INFO_ALBUM_ARTIST    "%album_artist"
#define INFO_ART_URL         "%art_url"
#define INFO_BITRATE         "%bitrate"
#define INFO_COMMENT         "%comment"

#define INFO_PLAYBACK_STATUS "%play_status"
#define INFO_SHUFFLE_MODE    "%shuffle"
#define INFO_VOLUME          "%volume"
#define INFO_LOOP_STATUS     "%loop_status"
#define INFO_POSITION        "%position"

#define INFO_FULL            "%full"

#define TRUE_LABEL      "true"
#define FALSE_LABEL     "false"

#define PLAYER_ACTIVE    "active"
#define PLAYER_INACTIVE  "inactive"

#define HELP_MESSAGE    "MPRIS control, version %s\n" \
"Usage:\n  %s [" ARG_PLAYER " <name,...> | " PLAYER_ACTIVE " | " PLAYER_INACTIVE "] COMMAND - Control running MPRIS player\n" \
"\n" \
"Options:\n" \
ARG_PLAYER " <name,...>\tExecute command only for player(s) named <name,...>\n" \
"         "PLAYER_ACTIVE"\t\tExecute command only for the active player(s) (default)\n" \
"         "PLAYER_INACTIVE"\tExecute command only for the inactive player(s)\n" \
"\n" \
"Commands:\n"\
"\t" CMD_HELP "\t\tThis help message\n" \
"\n" \
"\t" CMD_PLAY "\t\tBegin playing\n" \
"\t" CMD_PLAY_PAUSE "\t\tToggle play or pause\n" \
"\t" CMD_PAUSE "\t\tPause the player\n" \
"\t" CMD_STOP "\t\tStop the player\n" \
"\t" CMD_NEXT "\t\tChange track to the next in the playlist\n" \
"\t" CMD_PREVIOUS "\t\tChange track to the previous in the playlist\n" \
"\t" CMD_SEEK "\t\t[time[ms|s|m] Seek forwards or backwards in current track for 'time'.\n" \
"\t\t\tThe time can be a float value, if absent it defaults to 10 seconds.\n" \
"\t\t\tThe unit can be one of ms(milliseconds), s(seconds), m(minutes), if absent it defaults to seconds.\n" \
"\n" \
"\t" CMD_INFO "\t\t<format> Display information about the current track.\n" \
"\t\t\tThe default format is '%s'\n" \
"\t" CMD_STATUS "\t\tGet the playback status (equivalent to '" CMD_INFO " %" INFO_PLAYBACK_STATUS "')\n" \
"\t" CMD_LIST "\t\tGet the name of the running player(s) (equivalent to '" CMD_INFO " %" INFO_PLAYER_NAME "')\n" \
"\n" \
"Format specifiers for " CMD_INFO " command:\n" \
"\t%" INFO_PLAYER_NAME "\tprints the player name\n" \
"\t%" INFO_TRACK_NAME "\tprints the track name\n" \
"\t%" INFO_TRACK_NUMBER "\tprints the track number\n" \
"\t%" INFO_TRACK_LENGTH "\tprints the track length in seconds\n" \
"\t%" INFO_ARTIST_NAME "\tprints the artist name\n" \
"\t%" INFO_ALBUM_NAME "\tprints the album name\n" \
"\t%" INFO_ALBUM_ARTIST "\tprints the album artist\n" \
"\t%" INFO_ART_URL "\tprints the URL of the cover art image\n" \
"\t%" INFO_PLAYBACK_STATUS "\tprints the playback status\n" \
"\t%" INFO_SHUFFLE_MODE "\tprints the shuffle mode\n" \
"\t%" INFO_VOLUME "\t\tprints the volume\n" \
"\t%" INFO_LOOP_STATUS "\tprints the loop status\n" \
"\t%" INFO_POSITION "\tprints the song position in seconds\n" \
"\t%" INFO_BITRATE "\tprints the track's bitrate\n" \
"\t%" INFO_COMMENT "\tprints the track's comment\n" \
"\t%" INFO_FULL "\t\tprints all available information\n" \
""

const char* get_version(void)
{
#ifndef VERSION_HASH
#define VERSION_HASH "(unknown)"
#endif
    return VERSION_HASH;
}

const char* get_dbus_property_name (char* command)
{
    if (NULL == command) return NULL;
    if (strcmp(command, CMD_STATUS) == 0) {
        return MPRIS_PROP_PLAYBACK_STATUS;
    }
    if (strcmp(command, CMD_INFO) == 0) {
        return MPRIS_PROP_METADATA;
    }
    if (strcmp(command, CMD_LIST) == 0) {
        return INFO_PLAYER_NAME;
    }

    return NULL;
}

const char* get_dbus_method (char* command)
{
    if (NULL == command) return NULL;

    if (strcmp(command, CMD_PLAY) == 0) {
        return MPRIS_METHOD_PLAY;
    }
    if (strcmp(command,CMD_PAUSE) == 0) {
        return MPRIS_METHOD_PAUSE;
    }
    if (strcmp(command, CMD_STOP) == 0) {
        return MPRIS_METHOD_STOP;
    }
    if (strcmp(command, CMD_NEXT) == 0) {
        return MPRIS_METHOD_NEXT;
    }
    if (strcmp(command, CMD_PREVIOUS) == 0) {
        return MPRIS_METHOD_PREVIOUS;
    }
    if (strcmp(command, CMD_PLAY_PAUSE) == 0) {
        return MPRIS_METHOD_PLAY_PAUSE;
    }
    if (strcmp(command, CMD_SEEK) == 0) {
        return MPRIS_METHOD_SEEK;
    }
    if (strcmp(command, CMD_STATUS) == 0 || strcmp(command, CMD_INFO) == 0 || strcmp(command, CMD_LIST) == 0) {
        return DBUS_PROPERTIES_INTERFACE;
    }

    return NULL;
}

void print_help(char* name)
{
    const char* help_msg;
    const char* version = get_version();

    help_msg = HELP_MESSAGE;
    char* info_def = INFO_DEFAULT_STATUS;

    fprintf(stdout, help_msg, version, name, info_def);
}

void print_mpris_info(mpris_properties *props, const char* format)
{
    const char* info_full = INFO_FULL_STATUS;
    const char* shuffle_label = (props->shuffle ? TRUE_LABEL : FALSE_LABEL);
    char volume_label[5];
    snprintf(volume_label, 5, "%.2f", props->volume);
    char pos_label[11];
    snprintf(pos_label, 11, "%.2lfs", (props->position / 1000000.0));
    char track_number_label[6];
    snprintf(track_number_label, 6, "%d", props->metadata.track_number);
    char bitrate_label[6];
    snprintf(bitrate_label, 6, "%d", props->metadata.bitrate);
    char length_label[15];
    snprintf(length_label, 15, "%.2lfs", (props->metadata.length / 1000000.0));

    char output[MAX_OUTPUT_LENGTH*10];
    memcpy(output, format, strlen(format) + 1);

    str_replace(output, "\\n", "\n");
    str_replace(output, "\\t", "\t");

    str_replace(output, INFO_FULL, info_full);
    str_replace(output, INFO_PLAYER_NAME, props->player_name);
    str_replace(output, INFO_SHUFFLE_MODE, shuffle_label);
    str_replace(output, INFO_PLAYBACK_STATUS, props->playback_status);
    str_replace(output, INFO_VOLUME, volume_label);
    str_replace(output, INFO_LOOP_STATUS, props->loop_status);
    str_replace(output, INFO_POSITION, pos_label);
    str_replace(output, INFO_TRACK_NAME, props->metadata.title);
    str_replace(output, INFO_ARTIST_NAME, props->metadata.artist);
    str_replace(output, INFO_ALBUM_ARTIST, props->metadata.album_artist);
    str_replace(output, INFO_ALBUM_NAME, props->metadata.album);
    str_replace(output, INFO_TRACK_LENGTH, length_label);
    str_replace(output, INFO_TRACK_NUMBER, track_number_label);
    str_replace(output, INFO_BITRATE, bitrate_label);
    str_replace(output, INFO_COMMENT, props->metadata.comment);
    str_replace(output, INFO_ART_URL, props->metadata.art_url);

    fprintf(stdout, "%s\n", output);
}

#define DEFAULT_SKEEP_MSEC       5*1000 // 5 seconds

#define TIME_SUFFIX_SEC          "s"
#define TIME_SUFFIX_MIN          "m"
#define TIME_SUFFIX_MSEC         "ms"

int parse_time_argument(char *time_string)
{
    int ms = DEFAULT_SKEEP_MSEC;
    if (NULL == time_string) { return ms; }

    float time_units = -1.0;
    char suffix[10] = {0};

    int loaded = sscanf(time_string, "%f%s", &time_units, (char*)&suffix);
    if (loaded == 0 || loaded == EOF) return ms;
    if (loaded == 1) suffix[0] = 's';

    if (strncmp(suffix, TIME_SUFFIX_SEC, 1) == 0) {
        ms = time_units * 1000;
    }
    if (strncmp(suffix, TIME_SUFFIX_MIN, 1) == 0) {
        ms = time_units * 60 * 1000;
    }
    if (strncmp(suffix, TIME_SUFFIX_MSEC, 2) == 0) {
        ms = time_units;
    }

    return ms;
}

bool is_command(const char *param)
{
    if (NULL == param) return false;

    const char commands[11][7] = {CMD_HELP, CMD_PLAY, CMD_PAUSE, CMD_STOP, CMD_NEXT,
        CMD_PREVIOUS, CMD_PLAY_PAUSE, CMD_STATUS, CMD_SEEK, CMD_LIST, CMD_INFO};

    for (int i = 0; i <= (int)array_size(commands); i++) {
        const char *cmd = commands[i];
        if (strncmp(param, cmd, strlen(cmd)) == 0) {
                return true;
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    int status = EXIT_FAILURE;

    char* name = argv[0];
    if (argc <= 1) {
        goto _help;
    }
    bool show_help = false;
    bool active_players = false;
    bool inactive_players = false;
    char player_names[MAX_PLAYERS][MAX_OUTPUT_LENGTH] = {0};
    int player_count = 0;
    int ms = DEFAULT_SKEEP_MSEC;
    char **params = malloc(sizeof(char*)*(argc+1));
    int param_count = 0;

    char *info_format = INFO_DEFAULT_STATUS;
    char *command = NULL;
    for (int i = 1; i < argc; i++) {
        if (is_command(argv[i])) {
            command = argv[i];
            if (strncmp(command, CMD_HELP, strlen(CMD_HELP)) == 0) {
                show_help = true;
            } else if (strncmp(command, CMD_SEEK, strlen(CMD_SEEK)) == 0) {
                if (i <= argc) {
                    i++;
                    ms = parse_time_argument(argv[i]);
                }
            } else if (strncmp(command, CMD_INFO, strlen(CMD_INFO)) == 0 && argc > i+1) {
                info_format = argv[i+1];
            } else if (strncmp(command, CMD_STATUS, strlen(CMD_STATUS)) == 0) {
                info_format = INFO_PLAYBACK_STATUS;
            } else if (strncmp(command, CMD_LIST, strlen(CMD_LIST)) == 0) {
                info_format = INFO_PLAYER_NAME;
            }
        } else {
            params[param_count] = argv[i];
            param_count++;
        }
    }

    int option_index = 0;
    static struct option long_options[] = {
        {"player", required_argument, 0, 1},
        {"help", no_argument, 0, 2},
        {0, 0, 0, 0},
    };

    bool invalid_player_type = false;
    while (true) {
        int char_arg = getopt_long(param_count, params, "", long_options, &option_index);
        if (char_arg == -1) { break; }
        switch (char_arg) {
            case 1:
                if (strncmp(optarg, PLAYER_ACTIVE, strlen(PLAYER_ACTIVE)) == 0) {
                    active_players = true;
                    continue;
                }
                if (strncmp(optarg, PLAYER_INACTIVE, strlen(PLAYER_INACTIVE)) == 0) {
                    inactive_players = true;
                    continue;
                }
                optind--;
                for( ;optind < argc && *argv[optind] != '-'; optind++){
                    optarg = argv[optind];
                    if (
                        strncmp(optarg, CMD_INFO, strlen(CMD_INFO)) == 0 ||
                        strncmp(optarg, CMD_HELP, strlen(CMD_HELP)) == 0 ||
                        strncmp(optarg, CMD_STATUS, strlen(CMD_STATUS)) == 0 ||
                        strncmp(optarg, CMD_LIST, strlen(CMD_LIST)) == 0 ||
                        strncmp(optarg, CMD_NEXT, strlen(CMD_NEXT)) == 0 ||
                        strncmp(optarg, CMD_PAUSE, strlen(CMD_PAUSE)) == 0 ||
                        strncmp(optarg, CMD_PLAY, strlen(CMD_PLAY)) == 0 ||
                        strncmp(optarg, CMD_PLAY_PAUSE, strlen(CMD_PLAY_PAUSE)) == 0 ||
                        strncmp(optarg, CMD_PREVIOUS, strlen(CMD_PREVIOUS)) == 0 ||
                        strncmp(optarg, CMD_STOP, strlen(CMD_STOP)) == 0
                    ) {
                        break;
                    }
                    int len = strlen(optarg);
                    memcpy(player_names[player_count++], optarg, MIN(MAX_OUTPUT_LENGTH - 1, len));
                }
                if (invalid_player_type) {
                    fprintf(stderr, "Invalid player value '%s'\n", optarg);
                    goto _exit;
                }
            break;
            case 2:
                show_help = true;
            break;
            default:
                break;
        }
    }
    if (!active_players && !inactive_players && player_count == 0) {
        active_players = true;
        inactive_players = false;
    }

    if (show_help) {
        // TODO(marius): add help subjects for each command
        goto _help;
    }

    char *dbus_method = (char*)get_dbus_method(command);
    if (NULL == dbus_method) {
        //fprintf(stderr, "Invalid command %s (use help for help)\n", command);
        goto _exit;
    }

    // initialise the errors
    DBusError err = {0};
    dbus_error_init(&err);

    // connect to the system bus and check for errors
    DBusConnection *conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus connection error(%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        goto _exit;
    }

    mpris_player players[MAX_PLAYERS] = {0};
    int found = load_players(conn, players);
    for (int i = 0; i < found; i++) {
        mpris_player player = players[i];
        load_mpris_properties(conn, player.namespace, &player.properties);
        bool skip = true;
        if (active_players && (strncmp(player.properties.playback_status, MPRIS_METADATA_VALUE_PLAYING, 8) == 0)) {
            skip = false;
        }
        if (inactive_players &&
            (strncmp(player.properties.playback_status, MPRIS_METADATA_VALUE_PAUSED, 7) == 0 ||
            strncmp(player.properties.playback_status, MPRIS_METADATA_VALUE_STOPPED, 8) == 0)) {
            skip = false;
        }
        for (int i = 0; i < player_count; i++) {
            char *player_name = player_names[i];
            if (NULL != player_name) {
                size_t name_len = strlen(player_name);
                size_t prop_name_len = strlen(player.properties.player_name);
                size_t prop_ns_len = strlen(player.namespace);
                if (prop_name_len < name_len) {
                    prop_name_len = name_len ;
                }
                if (prop_ns_len < name_len) {
                    prop_ns_len = name_len;
                }
                if (strncmp(player.properties.player_name, player_name, prop_name_len) == 0 ||
                   strncmp(player.namespace, player_name, prop_ns_len) == 0) {
                    skip = false;
                }
            }
        }
        if (skip) {
            continue;
        }
        const char *dbus_property = (char*)get_dbus_property_name(command);
        if (NULL == dbus_property) {
            if (strncmp(command, CMD_SEEK, 4) == 0) {
                seek (conn, player, ms);
            } else {
                call_dbus_method(conn, player.namespace, MPRIS_PLAYER_PATH, MPRIS_PLAYER_INTERFACE, dbus_method);
            }
        } else {
            print_mpris_info(&player.properties, info_format);
        }
    }
    status = EXIT_SUCCESS;

    if (NULL != conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
_exit:
    return status;
_help:
        print_help(name);
        goto _exit;
}

