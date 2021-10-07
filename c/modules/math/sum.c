#include "sum.h"

__attribute__((visibility("protected")))
buffer run(result *res, buffer *buf) {
    INITIALIZE_BUFFER(buf_out);

    unsigned int a = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    unsigned int b = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Summing numbers: %u + %u", a, b)

    unsigned int result = a + b;

    buf_out = create_buffer(res, 4);
    HANDLE_ERROR_RESULT((*res))

    write_unsigned_int(res, &buf_out, result);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Summing result: = %u", result)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    return buf_out;
}
