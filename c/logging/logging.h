#pragma once

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

int start_logging();

int write_log(enum log_levels level, char const *fmt, ...);

int stop_logging();
