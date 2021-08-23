#pragma once

#include "logging/logging.h"

#include <errno.h>
#include <stdint.h>

enum result_code {
    SUCCESS = 0,

    FAILED_SOCKET = 1,
    FAILED_BIND = 2,
    FAILED_LISTEN = 3,
    FAILED_ACCEPT = 4,
    FAILED_READ = 5,
    FAILED_WRITE = 6,
    FAILED_MALLOC = 7,
    FAILED_STAT = 8,
    FAILED_OPEN = 9,

    BUFFER_READING_OVERFLOW = 101,
    HANDSHAKE_FAILED = 102,
    UNKNOWN_COMMAND = 103,
};

typedef struct result_s {
    enum result_code code;
    int errno_value;
} result;

#define INITIALIZE_RESULT(res) result res = {SUCCESS, 0}

#define RESET_RESULT(res) { \
    res.code = SUCCESS; \
    res.errno_value = 0; \
}

#define RESULT_FAILED(res) res.code != SUCCESS

#define RESULT_SUCCEEDED(res) res.code == SUCCESS

#define SET_ERROR(res, err) { \
    res.code = err; \
    res.errno_value = errno; \
}

#define HANDLE_ERROR(res, err, fmt, ...) { \
    SET_ERROR(res, err); \
    write_log(ERROR, fmt, __VA_ARGS__); \
    goto error_cleanup; \
}

#define HANDLE_ERROR_RESULT(res) { \
    if (RESULT_FAILED(res)) goto error_cleanup; \
}
