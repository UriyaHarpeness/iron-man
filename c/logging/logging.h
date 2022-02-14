#pragma once

#include "../functions/functions.h"
#include "../result.h"

/**
 * Start logging.
 *
 * Point the log's file descriptor to the correct one, optionally open a file for the logs.
 *
 * @return  The result of the function.
 */
result start_logging();

#ifdef DEBUG_BUILD

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/// The default log path when logging to file.
#define LOG_PATH "log.txt"

/// The log levels.
enum log_levels {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/// The enabled log level.
#define ENABLED_LEVEL DEBUG

/// If logging to stdout.
#define LOG_TO_STDOUT 1

/**
 * Write a log.
 *
 * @param level The log's level.
 * @param file  The file where the log was called from.
 * @param func  The func where the log was called from.
 * @param line  The line where the log was called from.
 * @param fmt   The format of the log.
 * @param ...   The arguments for the log formatting.
 * @return  The number of characters logged.
 */
int write_log(enum log_levels level, const char *file, const char *func, unsigned int line, char const *fmt, ...);

/**
 * Stop logging.
 *
 * Optionally close the log file.
 */
void stop_logging();

/// The WRITE_LOG macro, write a log with extra information (file, func, line).
#define WRITE_LOG(level, fmt, ...) { \
    write_log(level, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
}

#else // DEBUG_BUILD

/// Empty behaviors for release build.
#define WRITE_LOG(level, fmt, ...) {}
#define stop_logging() {}

#endif // DEBUG_BUILD
