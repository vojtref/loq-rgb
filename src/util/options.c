#include "util/options.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util/logger.h"

#define USAGE_STR                                                              \
	"Usage: loq-rgb [OPTIONS]\n"                                               \
	"Options:\n"                                                               \
	"\t-m <MODE>, --mode=<MODE>\n"                                             \
	"\t\tSets the RGB mode\n"                                                  \
	"\t\tMust be one of: static, breath, wave, smooth\n"                       \
	"\t\tdefault: static\n"                                                    \
	"\t-s <SPEED>, --speed=<SPEED>\n"                                          \
	"\t\tSets the speed of animated modes\n"                                   \
	"\t\tShould be in range 1-4, other values clamped\n"                       \
	"\t\tNote: ignored for mode static\n"                                      \
	"\t\tdefault: 1\n"                                                         \
	"\t-b <BRIGHTNESS>, --brightness=<BRIGHTNESS>\n"                           \
	"\t\tSets the brightness level, must be one of: low, high\n"               \
	"\t\tdefault: low\n"                                                       \
	"\t-1 <RGB>, --zone1=<RGB>\n"                                              \
	"\t-2 <RGB>, --zone2=<RGB>\n"                                              \
	"\t-3 <RGB>, --zone3=<RGB>\n"                                              \
	"\t-4 <RGB>, --zone4=<RGB>\n"                                              \
	"\t\tSets the color of the corresponding zone\n"                           \
	"\t\tRGB must be a valid hex code (may be prefixed with #)\n"              \
	"\t\tNote: ignored for modes wave, smooth\n"                               \
	"\t\tdefault: #ffffff\n"                                                   \
	"\t-v, --verbose\n"                                                        \
	"\t\tIncrease the output log verbosity\n"                                  \
	"\t\tMay be repeated multiple times to be even more verbose\n"             \
	"\t-h, --help\n"                                                           \
	"\t\tPrint this help message and exit"

// Defaults
options_t OPTIONS = {
    .verbosity  = LOG_LEVEL_WARNING,
    .mode       = RGB_MODE_STATIC,
    .speed      = 0x01,
    .brightness = BRIGHTNESS_LOW,

    .zone_rgb =
        {
            {0xff, 0xff, 0xff},
            {0xff, 0xff, 0xff},
            {0xff, 0xff, 0xff},
            {0xff, 0xff, 0xff},
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
