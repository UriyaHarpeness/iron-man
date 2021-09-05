#include "functions.h"

#ifdef RELEASE_BUILD

void load_all_functions() {
    void *handle = dlopen("libc.so.6", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();    /* Clear any existing error */

    const char *function_name_pointer = function_names;
    for (size_t index = 0; index < NUMBER_OF_FUNCTIONS; index++) {
        *functions[index] = dlsym(handle, function_name_pointer);
        function_name_pointer += function_name_lengths[index] + 1;
    }

    dlclose(handle);
}

#endif // RELEASE_BUILD
