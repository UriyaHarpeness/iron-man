#pragma once

#include "../result.h"

#include <malloc.h>
#include <stddef.h>

// todo: think about handling very large messages in separate buffers maybe.
typedef struct buffer_s {
    char *data;
    size_t size;
    size_t position;
} buffer;

void reuse_buffer(result *res, buffer *buf, size_t size);

buffer create_buffer(result *res, size_t size);

void destroy_buffer(buffer *buf);

#define INITIALIZE_BUFFER(buf) buffer buf = {NULL, 0, 0}
