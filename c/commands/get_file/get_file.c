#include "get_file.h"

buffer get_file(result *res, buffer *buf) {
    // todo: send length
    WRITE_LOG(INFO, "Getting file: %s", buf->data + buf->position)

    int file_fd = -1;
    struct stat st;
    INITIALIZE_BUFFER(buf_out);

    if (stat(buf->data + buf->position, &st) == -1) {
        HANDLE_ERROR((*res), FAILED_STAT, "Failed getting information of file: %s", buf->data + buf->position)
    }

    buf_out = create_buffer(res, st.st_size);
    HANDLE_ERROR_RESULT((*res))

    file_fd = open(buf->data + buf->position, O_RDONLY);
    if (file_fd == -1) {
        HANDLE_ERROR((*res), FAILED_OPEN, "Failed opening file: %s", buf->data + buf->position)
    }

    if (read(file_fd, buf_out.data, buf_out.size) != buf_out.size) {
        HANDLE_ERROR((*res), FAILED_READ, "Failed reading file: %s", buf->data + buf->position)
    }

    WRITE_LOG(INFO, "Got file: %s", buf->data + buf->position)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    close(file_fd);

    return buf_out;
}
