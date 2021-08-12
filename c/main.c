#include "communication/connection.h"
#include "logging/logging.h"

int main() {
    INITIAL_RESULT(res);

    start_logging();

    write_log(CRITICAL, "Started Iron Man");

    res = connect_();

    write_log(WARNING, "Connected");
    res = communicate();
    write_log(WARNING, "Disconnected");

    disconnect();

    write_log(CRITICAL, "Stopped Iron Man");

    stop_logging();
}
