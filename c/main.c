#include "commands/commands.h"
#include "communication/connection.h"
#include "logging/logging.h"

int main() {
    INITIAL_RESULT(res);

    start_logging();

    write_log(CRITICAL, "Started Iron Man");

    res = connect_();
    HANDLE_ERROR_RESULT(res)

    write_log(WARNING, "Connected");
    res = communicate();
    write_log(WARNING, "Disconnected");

    error_cleanup:

    disconnect();

    write_log(CRITICAL, "Stopped Iron Man");

    stop_logging();
}
