#pragma once

#include <stdint.h>

typedef enum : uint8_t {
	RGB_MODE_STATIC = 0x01,
	RGB_MODE_BREATH = 0x03,
	RGB_MODE_WAVE   = 0x04,
	RGB_MODE_SMOOTH = 0x06,
} rgb_mode_t;

typedef enum : uint8_t {
	BRIGHTNESS_LOW  = 0x01,
	BRIGHTNESS_HIGH = 0x02,
} brightness_t;

typedef struct {
	uint8_t verbosity;
	rgb_mode_t mode;
	uint8_t speed;
	brightness_t brightness;

	uint8_t zone_rgb[4][3];
} options_t;

extern options_t OPTIONS;

void parse_opts(int argc, char **argv);
