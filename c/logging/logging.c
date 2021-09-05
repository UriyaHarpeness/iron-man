#include "logging.h"

#ifdef DEBUG_BUILD

int logger_fd = -1;
char level_names[6][11] = {"TRACE     ",
                           "DEBUG     ",
                           "INFO      ",
                           "WARNING   ",
                           "ERROR ",
                           "CRITICAL  "};

int start_logging() {
#if LOG_TO_STDOUT
    logger_fd = 1;
#else
    logger_fd = open(LOG_PATH, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
#endif // LOG_TO_STDOUT
    return logger_fd;
}

int write_log(enum log_levels level, const char *file, const char *func, unsigned int line, char const *fmt, ...) {
    va_list args;
    int bytes_written;
    struct tm *parsed;
    time_t now;

    if (level < ENABLED_LEVEL) {
        return 0;
    }

    time(&now);
    parsed = localtime(&now);
    dprintf(logger_fd, "[%d/%02d/%02d %02d:%02d:%02d] %s", parsed->tm_year + 1900, parsed->tm_mon, parsed->tm_mday,
            parsed->tm_hour, parsed->tm_min, parsed->tm_sec, level_names[level]);
    if (level == ERROR) {
        dprintf(logger_fd, (errno != 0 ? "%3d " : "    "), errno);
    }
    bytes_written = dprintf(logger_fd, "| %s:%s:%d", file, func, line);
    for (; bytes_written < 80; bytes_written++) {
        dprintf(logger_fd, " ");
    }
    dprintf(logger_fd, " | ");
    va_start(args, fmt);
    bytes_written = vdprintf(logger_fd, fmt, args);
    va_end(args);
    dprintf(logger_fd, "\n");

    return bytes_written;
}

int stop_logging() {
#if !LOG_TO_STDOUT
    logger_fd = close(logger_fd);
#endif // !LOG_TO_STDOUT
    return logger_fd;
}

#endif // DEBUG_BUILD
