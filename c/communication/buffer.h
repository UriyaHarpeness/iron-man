#pragma once

#include <stddef.h>

#define BUFFER_SIZE 1024

typedef struct buffer_s {
    char data[BUFFER_SIZE];
    size_t size;
} buffer;
