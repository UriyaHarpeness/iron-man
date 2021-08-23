#include "buffer.h"

void reuse_buffer(result *res, buffer *buf, size_t size) {
    destroy_buffer(buf);
    buf->size = size;
    buf->data = malloc(buf->size);
    if (buf->data == NULL) {
        HANDLE_ERROR((*res), FAILED_MALLOC, "Failed allocating buffer: %d", errno)
    }

    goto cleanup;

    error_cleanup:

    destroy_buffer(buf);

    cleanup:

    return;
}

buffer create_buffer(result *res, size_t size) {
    INITIALIZE_BUFFER(buf);
    reuse_buffer(res, &buf, size);
    HANDLE_ERROR_RESULT((*res));

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf);

    cleanup:

    return buf;
}

void destroy_buffer(buffer *buf) {
    free(buf->data);
    buf->size = 0;
    buf->position = 0;
    buf->data = NULL;
}
