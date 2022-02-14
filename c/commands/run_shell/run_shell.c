#include "run_shell.h"

#define PARENT 0
#define CHILD 1
#define READ 0
#define WRITE 1

__attribute__((section(".run_shell")))
buffer run_shell(result *res, buffer *buf) {
    INITIALIZE_BUFFER(buf_out);
    INITIALIZE_BUFFER(input_buf);
    int fds[2][2];
    int new_pid = 0;
    int child_exit_status = -1;
    unsigned int input_length;
    const char **args = NULL;
    const char *input = NULL;

    // Read arguments.
    unsigned int command_length = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))
    const char *command = read_string(res, buf, command_length);
    HANDLE_ERROR_RESULT((*res))
    unsigned int args_num = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Running file: %s, with %u args", command, args_num)

    // Prepare arguments for the command.
    args = malloc_f(sizeof(char *) * (args_num + 2));
    if (args == NULL) {
        HANDLE_ERROR((*res), FAILED_MALLOC, "Failed allocating buffer", NULL)
    }

    args[0] = command;
    for (size_t arg_index = 0; arg_index < args_num; arg_index++) {
        unsigned int arg_length = read_unsigned_int(res, buf);
        HANDLE_ERROR_RESULT((*res))
        args[arg_index + 1] = read_string(res, buf, arg_length);
        HANDLE_ERROR_RESULT((*res))
    }
    args[args_num + 1] = NULL;

    // Create pipes for parent/child process communication.
    if (pipe_f(fds[CHILD]) == -1) {
        HANDLE_ERROR((*res), FAILED_PIPE, "Failed piping child fds", NULL)
    }
    if (pipe_f(fds[PARENT]) == -1) {
        HANDLE_ERROR((*res), FAILED_PIPE, "Failed piping parent fds", NULL)
    }

    // Create a child process.
    new_pid = fork_f();
    if (-1 == new_pid) {
        HANDLE_ERROR((*res), FAILED_FORK, "Failed spawning child process", NULL)
    }

    if (0 == new_pid) {
        // Child process.

        // Close unused file descriptors.
        close_f(fds[CHILD][WRITE]);
        close_f(fds[PARENT][READ]);
        close_f(STDOUT_FILENO);
        close_f(STDIN_FILENO);
        close_f(STDERR_FILENO);

        // Redirect stdin/stdout/stderr to parent process.
        if (dup2_f(fds[CHILD][READ], STDIN_FILENO) == -1) {
            WRITE_LOG(ERROR, "Failed dup2", NULL)
        }
        if (dup2_f(fds[PARENT][WRITE], STDOUT_FILENO) == -1) {
            WRITE_LOG(ERROR, "Failed dup2", NULL)
        }
        if (dup2_f(fds[PARENT][WRITE], STDERR_FILENO) == -1) {
            WRITE_LOG(ERROR, "Failed dup2", NULL)
        }

        // Execute the command with the arguments.
        execvp_f(command, (char *const *) args);
        WRITE_LOG(ERROR, "Failed executing file %s", command)
        exit_f(128);
    }

    // Parent process.

    // Close unused file descriptors.
    close_f(fds[CHILD][READ]);
    close_f(fds[PARENT][WRITE]);

    fd_set read_fds;
    struct timeval tv;
    int select_result;

    char command_output[4096 + 1];
    size_t len;
    uint8_t killed_child = 0;

    WRITE_LOG(INFO, "Created child process: %d", new_pid)

    // todo: maybe check with WIFEXITED(wstatus), but not necessarily.

    // Repeat until child process is dead or killed.
    while ((waitpid_f(new_pid, &child_exit_status, WNOHANG) != -1) && (killed_child == 0)) {
        while (1) {
            // Set up file descriptors and timeout for select.
            FD_ZERO(&read_fds);
            FD_SET(fds[PARENT][READ], &read_fds);
            FD_SET(connection_fd, &read_fds);
            tv.tv_sec = 0;
            tv.tv_usec = 1000000;

            // Check if there is new data from the connection or the child process.
            select_result = select_f(((fds[PARENT][READ] > connection_fd) ? fds[PARENT][READ] : connection_fd) + 1,
                                     &read_fds, NULL, NULL, &tv);

            if (-1 == select_result) {
                // Failed select.
                HANDLE_ERROR((*res), FAILED_SELECT, "Failed select", NULL)
            } else if (select_result) {
                // Found new data.
                if (FD_ISSET(fds[PARENT][READ], &read_fds)) {
                    // New data from child process.

                    // Read the data.
                    len = read_f(fds[PARENT][READ], command_output, 4096);
                    command_output[len] = 0;
                    WRITE_LOG(INFO, "Read output of %zu bytes", len)
                    WRITE_LOG(TRACE, "Read output: %s", command_output)

                    // Send the data.
                    *res = send_string(command_output, len);
                    HANDLE_ERROR_RESULT((*res))

                    // Get the child process' exit status if pipe is closed.
                    if (len == 0) {
                        waitpid_f(new_pid, &child_exit_status, WNOHANG);
                        WRITE_LOG(INFO, "Child process has exited with status: %d", WEXITSTATUS(child_exit_status))
                        killed_child = 1;
                        break;
                    }
                }
                if (FD_ISSET(connection_fd, &read_fds)) {
                    // New data from connection.

                    // Read the data.
                    *res = read_into_buffer(&input_buf);
                    HANDLE_ERROR_RESULT((*res))
                    input_length = read_unsigned_int(res, &input_buf);
                    HANDLE_ERROR_RESULT((*res))
                    if (0 == input_length) {
                        // Stop the child process.
                        WRITE_LOG(INFO, "Killing child process", NULL)
                        if (kill_f(new_pid, SIGTERM) == -1) {
                            HANDLE_ERROR((*res), FAILED_KILL, "Failed killing child process", NULL)
                        }
                        killed_child = 1;
                        break;
                    }

                    // Send the data to the child process.
                    input = read_string(res, &input_buf, input_length);
                    HANDLE_ERROR_RESULT((*res))
                    WRITE_LOG(INFO, "Writing input of %zu bytes", input_length)
                    WRITE_LOG(TRACE, "Writing input: %s", input)
                    write_f(fds[CHILD][WRITE], input, input_length);
                    write_f(fds[CHILD][WRITE], "\n", 1);
                }
            } else {
                // Timeout select.
                break;
            }
        }
    }

    // Write the child process exit code to the result buffer.
    *res = reuse_buffer(&buf_out, 1);
    HANDLE_ERROR_RESULT((*res))
    *res = write_uint8_t(&buf_out, WEXITSTATUS(child_exit_status));
    HANDLE_ERROR_RESULT((*res))

    WRITE_LOG(INFO, "Finished running file: %s", command)

    goto cleanup;

    error_cleanup:

    // Kill the child process.
    if (0 != new_pid) {
        kill_f(new_pid, SIGTERM);
        waitpid_f(new_pid, &child_exit_status, WNOHANG);
    }

    destroy_buffer(&buf_out);

    cleanup:

    // Close pipe file descriptors.
    close_f(fds[CHILD][WRITE]);
    close_f(fds[PARENT][READ]);

    destroy_buffer(&input_buf);

    // Free the command's arguments.
    free_f(args);

    return buf_out;
}
