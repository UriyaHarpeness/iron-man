#pragma once

#include "../functions/functions.h"
#include "../result.h"

#include <stddef.h>
#include <stdlib.h>

// todo: think about handling very large messages in separate buffers maybe.

/**
 * The buffer structure, defines a stream of characters with size and position.
 */
typedef struct buffer_s {
    /// The content of the stream.
    char *data;

    /// The total size of the stream.
    size_t size;

    /// The current position in the stream.
    size_t position;
} buffer;

/**
 * Reuse a buffer structure.
 *
 * Releases the old buffer's content and allocates new.
 *
 * @param buf   The buffer structure to reuse.
 * @param size  The size of the buffer.
 * @return  The result of the function.
 */
result reuse_buffer(buffer *buf, size_t size);

/**
 * Create a new buffer.
 *
 * @param[out] res  The result of the function.
 * @param[in] size  The size of the buffer.
 * @return  The buffer.
 */
buffer create_buffer(result *res, size_t size);

/**
 * Destroy a buffer.
 *
 * @param buf   The buffer to destroy.
 */
void destroy_buffer(buffer *buf);

/**
 * Convert a char* to uint64_t.
 *
 * @param buf   The value to convert.
 * @return  The value to converted to uint64_t.
 */
uint64_t char_to_uint64(const char buf[8]);

/**
 * Convert a uint64_t to char*.
 *
 * @param[in] value The value to convert.
 * @param[out] buf  The value converted to char*.
 */
void uint64_to_char(uint64_t value, char buf[8]);

/**
 * Convert a uint8_t to char*.
 *
 * @param[in] value The value to convert.
 * @param[out] buf  The value converted to char*.
 */
void uint8_to_char(uint8_t value, char buf[1]);

/**
 * Convert a char* to unsigned int.
 *
 * @param buf   The value to convert.
 * @return  The value to converted to unsigned int.
 */
unsigned int char_to_unsigned_int(const char buf[4]);

/**
 * Convert an unsigned int to char*.
 *
 * @param[in] value The value to convert.
 * @param[out] buf  The value converted to char*.
 */
void unsigned_int_to_char(unsigned int value, char buf[4]);

/**
 * Read a uint64_t from a buffer.
 *
 * @param[out] res  The result of the function.
 * @param[in] buf   The buffer to read from.
 * @return  The value read from the buffer.
 */
uint64_t read_uint64_t(result *res, buffer *buf);

/**
 * Read an unsigned int from a buffer.
 *
 * @param[out] res  The result of the function.
 * @param[in] buf   The buffer to read from.
 * @return  The value read from the buffer.
 */
unsigned int read_unsigned_int(result *res, buffer *buf);

/**
 * Read a char* from a buffer.
 *
 * @param[out] res      The result of the function.
 * @param[in] buf       The buffer to read from.
 * @param[in] length    The length of the char* to read.
 * @return  The value read from the buffer.
 */
const char *read_string(result *res, buffer *buf, size_t length);

/**
 * Write an unsigned int to a buffer.
 *
 * @param buf   The buffer to write into.
 * @param value The value to write to the buffer.
 * @return  The result of the function.
 */
result write_unsigned_int(buffer *buf, unsigned int value);

/**
 * Write a uint8_t to a buffer.
 *
 * @param buf   The buffer to write into.
 * @param value The value to write to the buffer.
 * @return  The result of the function.
 */
result write_uint8_t(buffer *buf, uint8_t value);

/// The INITIALIZE_BUFFER macro, creates a buffer with initial values.
#define INITIALIZE_BUFFER(buf) buffer buf = {NULL, 0, 0}
