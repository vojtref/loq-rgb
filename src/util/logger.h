#pragma once

typedef enum {
	LOG_LEVEL_SILENT  = 0,
	LOG_LEVEL_ERROR   = 1,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_INFO    = 3,
	LOG_LEVEL_DEBUG   = 4
} log_level_t;

void log_init(const char *log_file_path);
void log_term();

void log_set_level(log_level_t level);

void log_error(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_debug(const char *fmt, ...);

typedef void (*log_func_t)(const char *, ...);
