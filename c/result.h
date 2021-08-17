#pragma once

#include "logging/logging.h"

#include <errno.h>
#include <stdint.h>

enum result_code {
    SUCCESS,

    FAILED_SOCKET,
    FAILED_BIND,
    FAILED_LISTEN,
    FAILED_ACCEPT,
    FAILED_READ,
    FAILED_WRITE,
    FAILED_MALLOC,

    BUFFER_READING_OVERFLOW,
    HANDSHAKE_FAILED,
};

typedef struct result_s {
    enum result_code code;
    int errno_value;
} result;

#define INITIAL_RESULT(res) result res = {0, 0}

#define RESULT_FAILED(res) res.code != SUCCESS

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
