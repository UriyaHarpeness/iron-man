#include "sum.h"

__attribute__((visibility("protected")))
buffer difference(result *res, buffer *buf) {
    INITIALIZE_BUFFER(buf_out);

    // Read arguments.
    unsigned int a = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    unsigned int b = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Differencing numbers: |%u - %u|", a, b)

    // Calculate the difference.
    unsigned int result = a > b ? a - b : b - a;

    // Create a buffer and write the result into it.
    buf_out = create_buffer(res, 4);
    HANDLE_ERROR_RESULT((*res))

    *res = write_unsigned_int(&buf_out, result);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Difference result: = %u", result)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    return buf_out;
}
