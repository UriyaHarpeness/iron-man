#include "module.h"

__attribute__((visibility("protected")))
result module_constructor() {
    INITIALIZE_RESULT(res);

    res = load_all_functions();
    HANDLE_ERROR_RESULT(res)

    res = start_logging();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(INFO, "Loading module", NULL)

    error_cleanup:

    return res;
}

__attribute__((visibility("protected")))
void module_destructor() {
    WRITE_LOG(INFO, "Unloading module", NULL)
    stop_logging();
}
