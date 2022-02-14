#include "functions.h"

#ifdef RELEASE_BUILD

result load_all_functions() {
    INITIALIZE_RESULT(res);

    // Initialize the AES context used to encrypt the function names.
    struct AES_ctx functions_ctx;
    AES_init_ctx_iv(&functions_ctx, FUNCTION_NAMES_KEY, FUNCTION_NAMES_IV);

    // Open the shared object containing the functions.
    void *handle = dlopen(string_libc_so_6, RTLD_LAZY);
    if (!handle) {
        HANDLE_ERROR(res, FAILED_DLOPEN, "Failed opening shared object", NULL)
    }

    // Decrypt the function names.
    AES_CTR_xcrypt_buffer(&functions_ctx, (uint8_t *) function_names, FUNCTION_NAMES_LENGTH);

    // Iterate the function names and load each function's address to its matching pointer.
    const char *function_name_pointer = function_names;
    for (size_t index = 0; index < NUMBER_OF_FUNCTIONS; index++) {
        *functions[index] = dlsym(handle, function_name_pointer);
        if (*functions[index] == NULL) {
            HANDLE_ERROR(res, FAILED_DLSYM, "Failed loading symbol %s", function_name_pointer)
        }
        function_name_pointer += function_name_lengths[index] + 1;
    }

    goto cleanup;

    error_cleanup:

    cleanup:

    // Close the shared object.
    dlclose(handle);

    return res;
}

#else // RELEASE_BUILD

result load_all_functions() {
    INITIALIZE_RESULT(res);
    return res;
}

#endif // RELEASE_BUILD
