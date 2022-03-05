#pragma once

#include "buffer.h"
#include "../commands/commands.h"
#include "../consts.h"
#include "../functions/functions.h"
#include "../logging/logging.h"
#include "../result.h"
#include "../tiny-aes/aes.h"

#include <netdb.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/// The connection's file descriptor.
extern int connection_fd;

/**
 * Read data from the connection into a buffer.
 *
 * Reads the data from the connection, decrypts it, and saves it into the buffer.
 *
 * @param[out] buf  The buffer to read into.
 * @return  The result of the function.
 */
result read_into_buffer(buffer *buf);

/**
 * Read data from the connection into a buffer.
 *
 * @see read_into_buffer.
 *
 * @param[out] res  The result of the function.
 * @return  A buffer with the data from the connection.
 */
buffer read_buffer(result *res);

/**
 * Send a string to the connection.
 *
 * Encrypts the data and sends it to the connection.
 *
 * @param string    The string to send.
 * @param size      The size of the string.
 * @return  The result of the function.
 */
result send_string(char *string, uint64_t size);

/**
 * Send a buffer to the connection.
 *
 * @see send_string.
 *
 * @param buf   The buffer to send.
 * @return  The result of the function.
 */
result send_buffer(buffer buf);

/**
 * Send a result to the connection.
 *
 * @param res   The result to send.
 * @return  The result of the function.
 */
result send_result(result res);

/**
 * Accept a connection.
 *
 * Binds to a port, listens, accepts a connection, and validates a handshake.
 *
 * @return  The result of the function.
 */
result connect_();

/**
 * Communicate with the connection.
 *
 * The main connection loop, handles commands calling until disconnection.
 *
 * @param self_path The path of this executable.
 * @return  The result of the function.
 */
result communicate(const char *self_path);

/**
 * Disconnect the connection.
 */
void disconnect();
