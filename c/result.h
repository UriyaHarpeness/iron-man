#pragma once

#include <stdint.h>

enum result_code {
    SUCCESS,

    FAILED_SOCKET,
    FAILED_BIND,
    FAILED_LISTEN,
    FAILED_ACCEPT,
};

typedef struct result_s {
    enum result_code code;
    int errno_value;
} result;

#define INITIAL_RESULT(name) result name = {0, 0}
