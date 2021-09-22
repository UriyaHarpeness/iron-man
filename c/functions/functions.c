#include "functions.h"

#ifdef RELEASE_BUILD

result load_all_functions() {
    INITIALIZE_RESULT(res);

    struct AES_ctx command_ctx;
    AES_init_ctx_iv(&command_ctx, FUNCTION_NAMES_KEY, FUNCTION_NAMES_IV);
    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) libc_name, LIBC_NAME_LENGTH);

    void *handle = dlopen(libc_name, RTLD_LAZY);
    if (!handle) {
        HANDLE_ERROR(res, FAILED_DLOPEN, "Failed opening shared object", NULL)
    }

    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) function_names, FUNCTION_NAMES_LENGTH);
    const char *function_name_pointer = function_names;
    for (size_t index = 0; index < NUMBER_OF_FUNCTIONS; index++) {
        *functions[index] = dlsym(handle, function_name_pointer);
        if (*functions[index] == NULL) {
            HANDLE_ERROR(res, FAILED_DLSYM, "Failed loading symbol %s", function_name_pointer)
        }
        function_name_pointer += function_name_lengths[index] + 1;
    }

    error_cleanup:

    cleanup:

    dlclose(handle);

    return res;
}

#else // RELEASE_BUILD

result load_all_functions() {
    INITIALIZE_RESULT(res);
    return res;
}

#endif // RELEASE_BUILD
