#include "put_file.h"

buffer put_file(result *res, buffer *buf) {
    unsigned int path_length = read_unsigned_int(res, buf);
    write_log(INFO, "Putting file: %s", buf->data + buf->position);

    int file_fd = -1;
    INITIALIZE_BUFFER(buf_out);

    file_fd = open(buf->data + buf->position, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        HANDLE_ERROR((*res), FAILED_OPEN, "Failed opening file: %s", buf->data + buf->position)
    }

    if (write(file_fd, buf->data + buf->position + path_length, buf->size - buf->position - path_length) !=
        buf->size - buf->position - path_length) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed writing file: %s", buf->data + buf->position)
    }

    write_log(INFO, "Put file: %s", buf->data + buf->position);

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    close(file_fd);

    return buf_out;
}
