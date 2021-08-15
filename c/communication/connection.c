#include "connection.h"
#include "../tiny-aes/aes.h"

int socket_fd = -1;
int connection_fd = -1;

result connect_() {
    int client_address_len;
    struct sockaddr_in server_address, client_address;
    INITIAL_RESULT(res);

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == socket_fd) {
        write_log(ERROR, "Failed creating socket: %d", errno);
        goto error_cleanup;
    } else {
        write_log(DEBUG, "Created socket");
    }
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
        write_log(ERROR, "Failed binding socket: %d", errno);
        goto error_cleanup;
    } else
        write_log(DEBUG, "Bound socket");

    if (listen(socket_fd, 0) == -1) {
        write_log(ERROR, "Failed listening on socket: %d", errno);
        goto error_cleanup;
    } else {
        write_log(DEBUG, "Listening on socket");
    }
    client_address_len = sizeof(client_address);

    connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (-1 == connection_fd) {
        write_log(ERROR, "Failed accepting connection: %d", errno);
        goto error_cleanup;
    } else {
        write_log(DEBUG, "Accepted connection");
    }

    goto cleanup;

    error_cleanup:

    disconnect();

    cleanup:

    return res;
}

result communicate() {
    char buff[BUFFER_SIZE];
    INITIAL_RESULT(res);

    uint8_t key[32] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                       0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};
    uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);

    while (1) {
        bzero(buff, BUFFER_SIZE);

        if (read(connection_fd, buff, sizeof(buff)) <= 0) {
            write_log(ERROR, "Failed reading from socket: %d", errno);
            goto cleanup;
        }
        AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buff, strlen(buff));

        write_log(INFO, "Got \"%s\" from client", buff);

        AES_CTR_xcrypt_buffer(&ctx, (uint8_t *) buff, strlen(buff));

        if (write(connection_fd, buff, strlen(buff)) == -1) {
            write_log(ERROR, "Failed writing to socket: %d", errno);
            goto cleanup;
        }

        if (strncmp("exit", buff, 4) == 0) {
            write_log(INFO, "Gracefully disconnecting");
            goto cleanup;
        }
    }

    cleanup:

    return res;
}

void disconnect() {
    close(connection_fd);
    close(socket_fd);
}