#include "connection.h"

int socket_fd = -1;
int connection_fd = -1;

struct AES_ctx ctx;

uint64_t char_to_uint64(const char buf[8]) {
    return *((uint64_t *) buf);
}

void uint64_to_char(uint64_t value, char buf[8]) {
    *((uint64_t *) buf) = value;
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

void read_into_buffer(result *res, buffer *buf) {
    char buf_size[8];
    if (read(connection_fd, buf_size, 8) != 8) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading from socket: %d", errno)
    }
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buf_size, sizeof(buf_size));

    reuse_buffer(res, buf, char_to_uint64(buf_size));
    HANDLE_ERROR_RESULT((*res))

    if (read(connection_fd, buf->data, buf->size) != buf->size) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading from socket: %d", errno)
    }
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buf->data, buf->size);

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return;
}

buffer read_buffer(result *res) {
    buffer buf = {NULL, 0, 0};
    read_into_buffer(res, &buf);
    return buf;
}

result connect_() {
    unsigned int client_address_len;
    struct sockaddr_in server_address, client_address;
    INITIAL_RESULT(res);

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == socket_fd) {
        HANDLE_ERROR(res, FAILED_SOCKET, "Failed creating socket: %d", errno)
    }
    write_log(DEBUG, "Created socket");
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
        HANDLE_ERROR(res, FAILED_BIND, "Failed binding socket: %d", errno)
    }
    write_log(DEBUG, "Bound socket");

    if (listen(socket_fd, 0) == -1) {
        HANDLE_ERROR(res, FAILED_LISTEN, "Failed listening on socket: %d", errno)
    }
    write_log(DEBUG, "Listening on socket");

    client_address_len = sizeof(client_address);

    connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (-1 == connection_fd) {
        HANDLE_ERROR(res, FAILED_ACCEPT, "Failed accepting connection: %d", errno)
    }
    write_log(DEBUG, "Accepted connection");

    AES_init_ctx_iv(&ctx, KEY, IV);

    buffer buf = read_buffer(&res);
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

    return res;
}

result communicate() {
    buffer buf = {NULL, 0, 0};
    uint16_t conn = 1;
    INITIAL_RESULT(res);

    while (conn) {
        read_into_buffer(&res, &buf);
        HANDLE_ERROR_RESULT(res)

        write_log(INFO, "Got \"%s\" from client", buf.data);

        if (strncmp("exit", buf.data, 4) == 0) {
            conn = 0;
            write_log(INFO, "Gracefully disconnecting");
        }

        // todo: use buffer and send it.
        char size[8];
        uint64_to_char(buf.size, size);
        AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) size, 8);
        if (write(connection_fd, size, 8) != 8) {
            HANDLE_ERROR(res, FAILED_WRITE, "Failed writing to socket: %d", errno)
        }

        AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buf.data, buf.size);
        if (write(connection_fd, buf.data, buf.size) != buf.size) {
            HANDLE_ERROR(res, FAILED_WRITE, "Failed writing to socket: %d", errno)
        }
    }

    error_cleanup:

    cleanup:

    return res;
}

void disconnect() {
    close(connection_fd);
    close(socket_fd);
}
