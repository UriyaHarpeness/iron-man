#include "functions.h"

#ifdef RELEASE_BUILD

void load_all_functions() {
    void *handle = dlopen("libc.so.6", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();    /* Clear any existing error */

    struct AES_ctx command_ctx;
    AES_init_ctx_iv(&command_ctx, FUNCTION_NAMES_KEY, FUNCTION_NAMES_IV);
    AES_CTR_xcrypt_buffer(&command_ctx, (uint8_t *) function_names, FUNCTION_NAMES_LENGTH);

    const char *function_name_pointer = function_names;
    for (size_t index = 0; index < NUMBER_OF_FUNCTIONS; index++) {
        *functions[index] = dlsym(handle, function_name_pointer);
        function_name_pointer += function_name_lengths[index] + 1;
    }

    dlclose(handle);
}

#endif // RELEASE_BUILD
