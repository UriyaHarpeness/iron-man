#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

int main() {
    load_all_functions();

    INITIALIZE_RESULT(res);

    start_logging();

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
