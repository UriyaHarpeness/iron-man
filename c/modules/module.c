#include "module.h"

static uint8_t loads = 0;

__attribute__((visibility("protected")))
result module_constructor() {
    INITIALIZE_RESULT(res);

    if (loads == 0) {
        res = load_all_functions();
        HANDLE_ERROR_RESULT(res)

        res = start_logging();
        HANDLE_ERROR_RESULT(res)

        WRITE_LOG(INFO, "Loading module", NULL)
    }

    loads += 1;

    error_cleanup:

    return res;
}

__attribute__((visibility("protected")))
void module_destructor() {
    if (loads == 1) {
        WRITE_LOG(INFO, "Unloading module", NULL)
        stop_logging();
    }

    loads -= 1;
}
