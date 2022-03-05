#include "put_file.h"

__attribute__((section(".put_file")))
buffer put_file(result *res, buffer *buf) {
    INITIALIZE_BUFFER(buf_out);
    int file_fd = -1;

    unsigned int path_length = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Putting file: %s", buf->data + buf->position)

    // Open a new file to write.
    file_fd = open_f(buf->data + buf->position, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        HANDLE_ERROR((*res), FAILED_OPEN, "Failed opening file: %s", buf->data + buf->position)
    }

    // Write the content into the file.
    if (write_f(file_fd, buf->data + buf->position + path_length, buf->size - buf->position - path_length) !=
        buf->size - buf->position - path_length) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed writing file: %s", buf->data + buf->position)
    }

    WRITE_LOG(INFO, "Put file: %s", buf->data + buf->position)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    // Close the file.
    close_f(file_fd);

    return buf_out;
}
