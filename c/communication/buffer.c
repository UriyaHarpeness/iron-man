#include "buffer.h"

result reuse_buffer(buffer *buf, size_t size) {
    INITIALIZE_RESULT(res);

    // Release the old buffer's content.
    destroy_buffer(buf);

    // Prepare and allocate data for the new buffer.
    buf->size = size;
    buf->data = malloc_f(buf->size);
    if (buf->data == NULL) {
        HANDLE_ERROR(res, FAILED_MALLOC, "Failed allocating buffer", NULL)
    }

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return res;
}

buffer create_buffer(result *res, size_t size) {
    INITIALIZE_BUFFER(buf);

    // Create the new buffer.
    *res = reuse_buffer(&buf, size);
    HANDLE_ERROR_RESULT((*res));

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf);

    cleanup:

    return buf;
}

void destroy_buffer(buffer *buf) {
    // Free the buffer's data and zero the values.
    free_f(buf->data);
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

void uint8_to_char(uint8_t value, char buf[1]) {
    *((uint8_t *) buf) = value;
}

unsigned int char_to_unsigned_int(const char buf[4]) {
    return *((unsigned int *) buf);
}

void unsigned_int_to_char(unsigned int value, char buf[4]) {
    *((unsigned int *) buf) = value;
}

uint64_t read_uint64_t(result *res, buffer *buf) {
    uint64_t value = 0;

    // Make sure there is enough data in the buffer.
    if ((buf->size - buf->position) < 8) {
        HANDLE_ERROR((*res), BUFFER_READING_OVERFLOW, "Buffer reading overflow", NULL)
    }

    // Read the value and move the position.
    value = char_to_uint64(buf->data + buf->position);
    buf->position += 8;

    error_cleanup:

    return value;
}

unsigned int read_unsigned_int(result *res, buffer *buf) {
    unsigned int value = 0;

    // Make sure there is enough data in the buffer.
    if ((buf->size - buf->position) < 4) {
        HANDLE_ERROR((*res), BUFFER_READING_OVERFLOW, "Buffer reading overflow", NULL)
    }

    // Read the value and move the position.
    value = char_to_unsigned_int(buf->data + buf->position);
    buf->position += 4;

    error_cleanup:

    return value;
}

const char *read_string(result *res, buffer *buf, size_t length) {
    char *value = NULL;

    // Make sure there is enough data in the buffer.
    if ((buf->size - buf->position) < length) {
        HANDLE_ERROR((*res), BUFFER_READING_OVERFLOW, "Buffer reading overflow", NULL)
    }

    // Read the value and move the position.
    value = buf->data + buf->position;
    buf->position += length;

    error_cleanup:

    return value;
}

result write_unsigned_int(buffer *buf, unsigned int value) {
    INITIALIZE_RESULT(res);

    // Make sure there is enough data in the buffer.
    if ((buf->size - buf->position) < 4) {
        HANDLE_ERROR(res, BUFFER_WRITING_OVERFLOW, "Buffer writing overflow", NULL)
    }

    // Write the value and move the position.
    unsigned_int_to_char(value, buf->data + buf->position);
    buf->position += 4;

    error_cleanup:

    return res;
}

result write_uint8_t(buffer *buf, uint8_t value) {
    INITIALIZE_RESULT(res);

    // Make sure there is enough data in the buffer.
    if ((buf->size - buf->position) < 1) {
        HANDLE_ERROR(res, BUFFER_WRITING_OVERFLOW, "Buffer writing overflow", NULL)
    }

    // Write the value and move the position.
    uint8_to_char(value, buf->data + buf->position);
    buf->position += 1;

    error_cleanup:

    return res;
}
