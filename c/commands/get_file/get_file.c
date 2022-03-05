#include "get_file.h"

__attribute__((section(".get_file")))
buffer get_file(result *res, buffer *buf) {
    WRITE_LOG(INFO, "Getting file: %s", buf->data + buf->position)

    int file_fd = -1;
    struct stat st;
    INITIALIZE_BUFFER(buf_out);

    // Get the size of the file.
    if (__xstat_f(_STAT_VER_LINUX, buf->data + buf->position, &st) == -1) {
        HANDLE_ERROR((*res), FAILED_STAT, "Failed getting information of file: %s", buf->data + buf->position)
    }

    // Allocate buffer to fit the file's size.
    buf_out = create_buffer(res, st.st_size);
    HANDLE_ERROR_RESULT((*res))

    // Open the file for read.
    file_fd = open_f(buf->data + buf->position, O_RDONLY);
    if (file_fd == -1) {
        HANDLE_ERROR((*res), FAILED_OPEN, "Failed opening file: %s", buf->data + buf->position)
    }

    // Read the file's content straight into the allocated buffer.
    if (read_f(file_fd, buf_out.data, buf_out.size) != buf_out.size) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading file: %s", buf->data + buf->position)
    }

    WRITE_LOG(INFO, "Got file: %s", buf->data + buf->position)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    // Close the file.
    close_f(file_fd);

    return buf_out;
}
