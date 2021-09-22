#pragma once

#include "../functions/functions.h"
#include "../result.h"

result start_logging();

#ifdef DEBUG_BUILD

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define LOG_PATH "log.txt"

enum log_levels {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

#define ENABLED_LEVEL DEBUG
#define LOG_TO_STDOUT 1

int write_log(enum log_levels level, const char *file, const char *func, unsigned int line, char const *fmt, ...);

void stop_logging();

#define WRITE_LOG(level, fmt, ...) { \
    write_log(level, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
}

#else // DEBUG_BUILD

#define WRITE_LOG(level, fmt, ...) {}
#define stop_logging() {}

#endif // DEBUG_BUILD
