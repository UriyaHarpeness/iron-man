#include "connection.h"

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

    while (1) {
        bzero(buff, BUFFER_SIZE);

        if (read(connection_fd, buff, sizeof(buff)) <= 0) {
            write_log(ERROR, "Failed reading from socket: %d", errno);
            goto cleanup;
        }
        write_log(INFO, "Got \"%s\" from client", buff);

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