#include "buffer.h"

void reuse_buffer(result *res, buffer *buf, size_t size) {
    destroy_buffer(buf);
    buf->size = size;
    buf->data = malloc(buf->size);
    if (buf->data == NULL) {
        HANDLE_ERROR((*res), FAILED_MALLOC, "Failed allocating buffer: %d", errno)
    }

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return;
}

buffer create_buffer(result *res, size_t size) {
    INITIALIZE_BUFFER(buf);
    reuse_buffer(res, &buf, size);
    HANDLE_ERROR_RESULT((*res));

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf);

    cleanup:

    return buf;
}

void destroy_buffer(buffer *buf) {
    free(buf->data);
    buf->size = 0;
    buf->position = 0;
    buf->data = NULL;
}

uint64_t char_to_uint64(const char buf[8]) {
    return *((uint64_t *) buf);
}

void uint64_to_char(uint64_t value, char buf[8]) {
    *((uint64_t *) buf) = value;
}

unsigned int char_to_unsigned_int(const char buf[4]) {
    return *((unsigned int *) buf);
}

void unsigned_int_to_char(unsigned int value, char buf[4]) {
    *((unsigned int *) buf) = value;
}

uint64_t read_uint64_t(result *res, buffer *buf) {
    uint64_t value = 0;
    if ((buf->size - buf->position) < 8) {
        HANDLE_ERROR((*res), BUFFER_READING_OVERFLOW, "Buffer reading overflow", NULL)
    }

    value = char_to_uint64(buf->data + buf->position);
    buf->position += 8;

    error_cleanup:

    return value;
}

unsigned int read_unsigned_int(result *res, buffer *buf) {
    unsigned int value = 0;
    if ((buf->size - buf->position) < 4) {
        HANDLE_ERROR((*res), BUFFER_READING_OVERFLOW, "Buffer reading overflow", NULL)
    }

    value = char_to_unsigned_int(buf->data + buf->position);
    buf->position += 4;

    error_cleanup:

    return value;
}
