#include "module.h"

/// The reference count of the module.
static uint8_t reference_count = 0;

__attribute__((visibility("protected")))
result module_constructor() {
    INITIALIZE_RESULT(res);

    // When the module is first loaded, load all the functions it needs and initialize logging.
    if (reference_count == 0) {
        res = load_all_functions();
        HANDLE_ERROR_RESULT(res)

        res = start_logging();
        HANDLE_ERROR_RESULT(res)

        WRITE_LOG(INFO, "Loading module", NULL)
    }

    // Increase the reference count.
    reference_count += 1;

    goto cleanup;

    error_cleanup:

    cleanup:

    return res;
}

__attribute__((visibility("protected")))
void module_destructor() {
    // When the module is lastly unloaded, stop the logging.
    if (reference_count == 1) {
        WRITE_LOG(INFO, "Unloading module", NULL)
        stop_logging();
    }

    // Decrease the reference count.
    reference_count -= 1;
}
