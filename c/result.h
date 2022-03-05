#pragma once

#include <errno.h>
#include <stdint.h>

/// The result codes.
enum result_code {
    /// Success.
    SUCCESS = 0,

    /// System/library calls failures.
    FAILED_SOCKET = 1,
    FAILED_BIND = 2,
    FAILED_LISTEN = 3,
    FAILED_ACCEPT = 4,
    FAILED_READ = 5,
    FAILED_WRITE = 6,
    FAILED_MALLOC = 7,
    FAILED_STAT = 8,
    FAILED_OPEN = 9,
    FAILED_PIPE = 10,
    FAILED_FORK = 11,
    FAILED_DUP2 = 12,
    FAILED_EXECVP = 13,
    FAILED_SELECT = 14,
    FAILED_KILL = 15,
    FAILED_SYSCONF = 16,
    FAILED_MPROTECT = 17,
    FAILED_DLOPEN = 18,
    FAILED_DLSYM = 19,
    FAILED_UNLINK = 20,

    /// Other high level errors and codes.
    BUFFER_READING_OVERFLOW = 101,
    BUFFER_WRITING_OVERFLOW = 102,
    HANDSHAKE_FAILED = 103,
    UNKNOWN_COMMAND = 104,
    STOPPING = 105,
    SUICIDING = 106,
};

/**
 * The result structure, defines the result of an operation.
 */
typedef struct result_s {
    /// The result code of the operation.
    enum result_code code;

    /// The errno value of the operation.
    int errno_value;
} result;

/// The INITIALIZE_RESULT macro, creates a result with initial values.
#define INITIALIZE_RESULT(res) result res = {SUCCESS, 0}

/// The RESET_RESULT macro, resets a result to the initial values.
#define RESET_RESULT(res) { \
    res.code = SUCCESS; \
    res.errno_value = 0; \
}

/// The RESULT_FAILED macro, checks if a result is a failure.
#define RESULT_FAILED(res) res.code != SUCCESS

/// The RESULT_SUCCEEDED macro, checks if a result is a success.
#define RESULT_SUCCEEDED(res) res.code == SUCCESS

// todo: try and lose errno and all the libc symbols.

/// The SET_ERROR macro, sets the result code and errno value of a result.
#define SET_ERROR(res, err) { \
    res.code = err; \
    res.errno_value = errno; \
}

/// The HANDLE_ERROR macro, sets the result code's value, writes a log with the error, and goes to error_cleanup.
#define HANDLE_ERROR(res, err, fmt, ...) { \
    SET_ERROR(res, err); \
    WRITE_LOG(ERROR, fmt, __VA_ARGS__); \
    goto error_cleanup; \
}

/// The HANDLE_ERROR_RESULT macro, if the result is a failure - goes to error_cleanup.
#define HANDLE_ERROR_RESULT(res) { \
    if (RESULT_FAILED(res)) goto error_cleanup; \
}
