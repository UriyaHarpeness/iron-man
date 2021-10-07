#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

// todo: not here
#include "strings/strings.h"

int main() {
    INITIALIZE_RESULT(res);

    res = load_all_functions();
    HANDLE_ERROR_RESULT(res)

    res = start_logging();
    HANDLE_ERROR_RESULT(res)

    res = initialize_commands();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(CRITICAL, "Started Iron Man", NULL)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    decrypt_strings();

    void *handle = dlopen("/c/projects/iron-man/c/cmake-build-debug/libsum.so", RTLD_LAZY);
    if (!handle) {
        HANDLE_ERROR(res, FAILED_DLOPEN, "Failed opening shared object %s", dlerror())
    }
    buffer (*chosen_command)(result *, buffer *) = NULL;
    int *(*eee)(void) = NULL;
    eee = dlsym(handle, "__errno_location");

    result (*module_constructor)();
    void (*module_destructor)();
    module_constructor = dlsym(handle, string_module_constructor);
    module_destructor = dlsym(handle, string_module_destructor);

    res = module_constructor();
    HANDLE_ERROR_RESULT(res)

    chosen_command = dlsym(handle, string_run);
    if (chosen_command == NULL) {
        HANDLE_ERROR(res, FAILED_DLSYM, "Failed loading symbol %s", "run")
    }

    INITIALIZE_BUFFER(b);
    b = create_buffer(&res, 8);
    write_unsigned_int(&res, &b, 4);
    write_unsigned_int(&res, &b, 4);
    b.position = 0;
    buffer t = chosen_command(&res, &b);
    module_destructor();
    dlclose(handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    res = connect_();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(WARNING, "Connected", NULL)
    res = communicate();
    WRITE_LOG(WARNING, "Disconnected", NULL)

    error_cleanup:

    disconnect();

    WRITE_LOG(CRITICAL, "Stopped Iron Man", NULL)

    destroy_commands();

    stop_logging();

    return EXIT_SUCCESS;
}
