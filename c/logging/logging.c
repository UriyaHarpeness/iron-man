#include "logging.h"

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

int write_log(enum log_levels level, char const *fmt, ...) {
    va_list args;
    int length;
    struct tm *parsed;
    time_t now;

    if (level < ENABLED_LEVEL) {
        return 0;
    }

    time(&now);
    parsed = localtime(&now);
    // todo: use macro to add the filename, function, and line number to logs.
    dprintf(logger_fd, "[%d/%02d/%02d %02d:%02d:%02d] %s", parsed->tm_year + 1900, parsed->tm_mon, parsed->tm_mday,
            parsed->tm_hour, parsed->tm_min, parsed->tm_sec, level_names[level]);
    if (level == ERROR) {
        dprintf(logger_fd, (errno != 0 ? "%3d " : "    "), errno);
    }
    dprintf(logger_fd, "| ");
    va_start(args, fmt);
    length = vdprintf(logger_fd, fmt, args);
    va_end(args);
    dprintf(logger_fd, "\n");

    return length;
}

int stop_logging() {
#if !LOG_TO_STDOUT
    logger_fd = close(logger_fd);
#endif // !LOG_TO_STDOUT
    return logger_fd;
}