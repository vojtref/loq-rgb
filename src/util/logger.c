#include "util/logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>


static FILE *log_file        = nullptr;
static log_level_t log_level = LOG_LEVEL_WARNING;


void log_init(const char *log_file_path)
{
	if (log_file_path == nullptr) return;

	if (log_file != nullptr) {
		fclose(log_file);
		log_file = nullptr;
	}

	log_file = fopen(log_file_path, "w");
	if (log_file != nullptr) {
		setbuf(log_file, nullptr);
	}
}


void log_term(void)
{
	if (log_file == nullptr) return;

	fclose(log_file);
	log_file = nullptr;
}


void log_set_level(log_level_t level)
{
	log_level = level;
}


static void log_write(FILE *file,
                      const char *label,
                      struct timespec ts,
                      const char *fmt,
                      va_list args)
{
	fprintf(file, "[%10jd.%03ld]%s: ", ts.tv_sec, ts.tv_nsec / 1000000, label);
	vfprintf(file, fmt, args);
	fputc('\n', file);
}


#define LOG_FUNC(fn_name, target_level, stderr_label, log_file_label)          \
	void fn_name(const char *fmt, ...)                                         \
	{                                                                          \
		if (log_level < (target_level)) return;                                \
                                                                               \
		struct timespec ts = {0};                                              \
		timespec_get(&ts, TIME_UTC);                                           \
                                                                               \
		va_list args;                                                          \
		va_start(args, fmt);                                                   \
                                                                               \
		if (log_file != nullptr) {                                             \
			va_list args_copy;                                                 \
			va_copy(args_copy, args);                                          \
			log_write(log_file, log_file_label, ts, fmt, args_copy);           \
			va_end(args_copy);                                                 \
		}                                                                      \
                                                                               \
		log_write(stderr, stderr_label, ts, fmt, args);                        \
		va_end(args);                                                          \
	}

LOG_FUNC(log_error,
         LOG_LEVEL_ERROR,
         "\033[1;91m!!!   ERROR\033[0m",
         "!!!   ERROR")
LOG_FUNC(log_warning,
         LOG_LEVEL_WARNING,
         "\033[1;93m !  WARNING\033[0m",
         " !  WARNING")
LOG_FUNC(log_info,
         LOG_LEVEL_INFO,
         "\033[1;96m i     INFO\033[0m",
         " i     INFO")
LOG_FUNC(log_debug,
         LOG_LEVEL_DEBUG,
         "\033[1;95m ?    DEBUG\033[0m",
         " ?    DEBUG")

#undef LOG_FUNC
