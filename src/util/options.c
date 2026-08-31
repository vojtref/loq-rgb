#include "util/options.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util/logger.h"


#define DEFAULT_VERBOSITY       LOG_LEVEL_WARNING
#define DEFAULT_MODE            RGB_MODE_STATIC
#define DEFAULT_MODE_NAME       "static"
#define DEFAULT_SPEED           1
#define DEFAULT_BRIGHTNESS      BRIGHTNESS_LOW
#define DEFAULT_BRIGHTNESS_NAME "low"
#define DEFAULT_COLOR(X)        X(ff, ff, ff)

// Preprocessor helpers
#define STR_(X)              #X
#define STR(X)               STR_(X)
#define COLOR_BYTES(R, G, B) {0x##R, 0x##G, 0x##B}
#define COLOR_STR(R, G, B)   "#" #R #G #B

// clang-format off
#define USAGE_STR                                                              \
	"Usage: loq-rgb [OPTIONS]\n"                                               \
	"Options:\n"                                                               \
	"    -m<MODE>, --mode=<MODE>\n"                                            \
	"        Sets the RGB mode\n"                                              \
	"        <MODE> must be one of: static, breath, wave, smooth\n"            \
	"        Default: " DEFAULT_MODE_NAME "\n"                                 \
	"    -s<SPEED>, --speed=<SPEED>\n"                                         \
	"        Sets the speed of animated modes\n"                               \
	"        <SPEED> should be an integer in the range 1-4,\n"                 \
	"        other values will be clamped to that range\n"                     \
	"        Note: does nothing in mode static\n"                              \
	"        Default: " STR(DEFAULT_SPEED) "\n"                                \
	"    -b<BRIGHTNESS>, --brightness=<BRIGHTNESS>\n"                          \
	"        Sets the brightness level\n"                                      \
	"        <BRIGHTNESS> must be one of: low, high\n"                         \
	"        Default: " DEFAULT_BRIGHTNESS_NAME "\n"                           \
	"    -1<COLOR>, --zone1=<COLOR>\n"                                         \
	"    -2<COLOR>, --zone2=<COLOR>\n"                                         \
	"    -3<COLOR>, --zone3=<COLOR>\n"                                         \
	"    -4<COLOR>, --zone4=<COLOR>\n"                                         \
	"        Sets the color of the corresponding zone\n"                       \
	"        <COLOR> must be a valid RGB hex code\n"                           \
	"        <COLOR> may be prefixed with #, also accepted without prefix\n"   \
	"        Note: does nothing in modes wave, smooth\n"                       \
	"        Default: " DEFAULT_COLOR(COLOR_STR) "\n"                          \
	"    -v, --verbose\n"                                                      \
	"        Increases output log verbosity\n"                                 \
	"        May be repeated multiple times\n"                                 \
	"    -h, --help\n"                                                         \
	"        Print this help message and exit"
// clang-format on


options_t OPTIONS = {
    .verbosity  = DEFAULT_VERBOSITY,
    .mode       = DEFAULT_MODE,
    .speed      = DEFAULT_SPEED,
    .brightness = DEFAULT_BRIGHTNESS,

    .zone_rgb =
        {
            DEFAULT_COLOR(COLOR_BYTES),
            DEFAULT_COLOR(COLOR_BYTES),
            DEFAULT_COLOR(COLOR_BYTES),
            DEFAULT_COLOR(COLOR_BYTES),
        },
};


static int parse_rgb(const char *s, uint8_t rgb[3])
{
	if (*s == '#') s++;

	if (strlen(s) != 6) return -1;

	char *end;
	unsigned long v = strtoul(s, &end, 16);

	if (*end != '\0' || v > 0xffffff) return -1;

	rgb[0] = (v >> 16) & 0xff;
	rgb[1] = (v >> 8) & 0xff;
	rgb[2] = v & 0xff;

	return 0;
}


void parse_opts(int argc, char **argv)
{
	static struct option long_options[] = {
	    {"mode", required_argument, nullptr, 'm'},
	    {"speed", required_argument, nullptr, 's'},
	    {"brightness", required_argument, nullptr, 'b'},
	    {"zone1", required_argument, nullptr, '1'},
	    {"zone2", required_argument, nullptr, '2'},
	    {"zone3", required_argument, nullptr, '3'},
	    {"zone4", required_argument, nullptr, '4'},
	    {"help", required_argument, nullptr, 'h'},
	    {"verbose", required_argument, nullptr, 'v'},
	    {nullptr, 0, nullptr, 0},
	};

	int c;
	while ((c = getopt_long(argc,
	                        argv,
	                        "m:s:b:1:2:3:4:hv",
	                        long_options,
	                        nullptr)) != -1) {
		switch (c) {
		case 'v':
			OPTIONS.verbosity++;
			log_set_level(OPTIONS.verbosity);
			break;
		case '1':
		case '2':
		case '3':
		case '4':
			if (parse_rgb(optarg, OPTIONS.zone_rgb[c - '1']) < 0) {
				log_warning("Invalid RGB for zone %c, ignoring: %s", c, optarg);
			}
			break;
		case 'm':
			if (strcmp(optarg, "static") == 0) {
				OPTIONS.mode = RGB_MODE_STATIC;
			} else if (strcmp(optarg, "breath") == 0) {
				OPTIONS.mode = RGB_MODE_BREATH;
			} else if (strcmp(optarg, "wave") == 0) {
				OPTIONS.mode = RGB_MODE_WAVE;
			} else if (strcmp(optarg, "smooth") == 0) {
				OPTIONS.mode = RGB_MODE_SMOOTH;
			} else {
				log_warning("Invalid mode, ignoring: %s", optarg);
			}
			break;
		case 's':
			int speed = atoi(optarg);
			if (speed < 1) {
				log_warning("Speed %d below minimum 1, clamping", speed);
				speed = 1;
			} else if (speed > 4) {
				log_warning("Speed %d above maximum 4, clamping", speed);
				speed = 4;
			}
			OPTIONS.speed = speed;
			break;
		case 'b':
			if (strcmp(optarg, "low") == 0) {
				OPTIONS.brightness = BRIGHTNESS_LOW;
			} else if (strcmp(optarg, "high") == 0) {
				OPTIONS.brightness = BRIGHTNESS_HIGH;
			} else {
				log_warning("Invalid brightness, ignoring: %s", optarg);
			}
			break;
		case 'h':
			printf("%s\n", USAGE_STR);
			exit(0);
		default:
		}
	}
}
