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

uint64_t char_to_uint64(const char buf[8]);

void uint64_to_char(uint64_t value, char buf[8]);

unsigned int char_to_unsigned_int(const char buf[4]);

void unsigned_int_to_char(unsigned int value, char buf[4]);

uint64_t read_uint64_t(result *res, buffer *buf);

unsigned int read_unsigned_int(result *res, buffer *buf);

#define INITIALIZE_BUFFER(buf) buffer buf = {NULL, 0, 0}
