#include "logging.h"

#ifdef DEBUG_BUILD

/// The log's file descriptor.
int logger_fd = -1;

/// The string representation of the different log levels.
static const char level_names[6][11] = {"TRACE     ",
                                        "DEBUG     ",
                                        "INFO      ",
                                        "WARNING   ",
                                        "ERROR ",
                                        "CRITICAL  "};

result start_logging() {
    INITIALIZE_RESULT(res);
#if LOG_TO_STDOUT
    // Point to stdout.
    logger_fd = STDOUT_FILENO;
#else
    // Open the log file.
    logger_fd = open_f(LOG_PATH, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
#endif // LOG_TO_STDOUT

    if (logger_fd == -1) {
        SET_ERROR(res, FAILED_OPEN)
    }
    return res;
}

int write_log(enum log_levels level, const char *file, const char *func, unsigned int line, char const *fmt, ...) {
    va_list args;
    int bytes_written;
    struct tm *parsed;
    time_t now;

    // Skip if level is not enabled.
    if (level < ENABLED_LEVEL) {
        return 0;
    }

    // Get and log the current time, and the log level.
    time(&now);
    parsed = localtime(&now);
    dprintf(logger_fd, "[%d/%02d/%02d %02d:%02d:%02d] %s", parsed->tm_year + 1900, parsed->tm_mon, parsed->tm_mday,
            parsed->tm_hour, parsed->tm_min, parsed->tm_sec, level_names[level]);

    // Log the value of errno for log level ERROR.
    if (level == ERROR) {
        dprintf(logger_fd, (errno != 0 ? "%3d " : "    "), errno);
    }

    // Log the location of the log call: file, function, and line.
    bytes_written = dprintf(logger_fd, "| %s:%s:%d", file, func, line);
    for (; bytes_written < 80; bytes_written++) {
        dprintf(logger_fd, " ");
    }
    dprintf(logger_fd, " | ");

    // Log the message with its arguments.
    va_start(args, fmt);
    bytes_written = vdprintf(logger_fd, fmt, args);
    va_end(args);
    dprintf(logger_fd, "\n");

    return bytes_written;
}

void stop_logging() {
#if !LOG_TO_STDOUT
    // Close the log file.
    close_f(logger_fd);
    logger_fd = -1;
#endif // !LOG_TO_STDOUT
}

#else // DEBUG_BUILD

result start_logging() {
    INITIALIZE_RESULT(res);
    return res;
}

#endif // DEBUG_BUILD
