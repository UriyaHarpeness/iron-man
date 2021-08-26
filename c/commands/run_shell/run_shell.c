#include "run_shell.h"

#define PARENT 0
#define CHILD 1
#define READ 0
#define WRITE 1

buffer run_shell(result *res, buffer *buf) {
    INITIALIZE_BUFFER(buf_out);
    INITIALIZE_BUFFER(input_buf);
    int fds[2][2];
    int new_pid;
    int status;
    unsigned int input_length;
    const char **args = NULL;
    const char *input = NULL;

    // Reading arguments.
    unsigned int command_length = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))
    const char *command = read_string(res, buf, command_length);
    HANDLE_ERROR_RESULT((*res))
    unsigned int args_num = read_unsigned_int(res, buf);
    HANDLE_ERROR_RESULT((*res))

    write_log(INFO, "Running file: %s, with %u args", command, args_num);

    // Preparing arguments for the command.
    args = malloc(sizeof(char *) * (args_num + 2));
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

    if (pipe(fds[CHILD]) == -1) {
        HANDLE_ERROR((*res), FAILED_PIPE, "Failed piping child fds", NULL)
    }
    if (pipe(fds[PARENT]) == -1) {
        HANDLE_ERROR((*res), FAILED_PIPE, "Failed piping parent fds", NULL)
    }

    new_pid = fork();
    if (-1 == new_pid) {
        HANDLE_ERROR((*res), FAILED_FORK, "Failed spawning child process", NULL)
    }

    if (0 == new_pid) {
        // Child process.
        close(fds[CHILD][WRITE]);
        close(fds[PARENT][READ]);
        close(STDOUT_FILENO);
        close(STDIN_FILENO);
        close(STDERR_FILENO);
        if (dup2(fds[CHILD][READ], STDIN_FILENO) == -1) {
            write_log(ERROR, "Failed dup2");
        }
        if (dup2(fds[PARENT][WRITE], STDOUT_FILENO) == -1) {
            write_log(ERROR, "Failed dup2");
        }
        if (dup2(fds[PARENT][WRITE], STDERR_FILENO) == -1) {
            write_log(ERROR, "Failed dup2");
        }

        execvp(command, (char *const *) args);
        write_log(ERROR, "Failed executing file %s", command);
        exit(EXIT_FAILURE);
    }

    // Parent process.
    close(fds[CHILD][READ]);
    close(fds[PARENT][WRITE]);

    fd_set read_fds;
    struct timeval tv;
    int select_result;

    char command_output[4096 + 1];
    size_t len;
    uint8_t killed_child = 0;

    write_log(INFO, "Created child process: %d", new_pid);

    // todo: maybe check with WIFEXITED(wstatus), but not necessarily.
    while ((waitpid(new_pid, &status, WNOHANG) != -1) && (killed_child == 0)) {
        tv.tv_sec = 0;
        tv.tv_usec = 1000000;
        while (1) {
            FD_ZERO(&read_fds);
            FD_SET(fds[PARENT][READ], &read_fds);
            FD_SET(connection_fd, &read_fds);
            select_result = select(((fds[PARENT][READ] > connection_fd) ? fds[PARENT][READ] : connection_fd) + 1,
                                   &read_fds, NULL, NULL, &tv);

            if (-1 == select_result) {
                HANDLE_ERROR((*res), FAILED_SELECT, "Failed select", NULL)
            } else if (select_result) {
                if (FD_ISSET(fds[PARENT][READ], &read_fds)) {
                    len = read(fds[PARENT][READ], command_output, 4096);
                    command_output[len] = 0;
                    write_log(INFO, "Read output of %zu bytes: %s", len, command_output);
                    *res = send_string(command_output, len);
                    HANDLE_ERROR_RESULT((*res))
                    if (len == 0 && waitpid(new_pid, &status, WNOHANG) != -1) {
                        write_log(INFO, "Child process has died %d", status);
                        killed_child = 1;
                        break;
                    }
                }
                if (FD_ISSET(connection_fd, &read_fds)) {
                    read_into_buffer(res, &input_buf);
                    HANDLE_ERROR_RESULT((*res))
                    input_length = read_unsigned_int(res, &input_buf);
                    HANDLE_ERROR_RESULT((*res))
                    if (0 == input_length) {
                        // Stop the child process.
                        write_log(INFO, "Killing child process");
                        if (kill(new_pid, SIGTERM) == -1) {
                            HANDLE_ERROR((*res), FAILED_KILL, "Failed killing child process", NULL)
                        }
                        killed_child = 1;
                        break;
                    }

                    input = read_string(res, &input_buf, input_length);
                    HANDLE_ERROR_RESULT((*res))
                    write_log(INFO, "Writing input of %zu bytes: %s", input_length, input);
                    write(fds[CHILD][WRITE], input, input_length);
                    write(fds[CHILD][WRITE], "\n", 1);
                }
            } else {
                break;
            }

            tv.tv_sec = 0;
            tv.tv_usec = 100000;
        }
    }

    write_log(INFO, "Finished running file: %s", command);

    error_cleanup:

    destroy_buffer(&buf_out);

    cleanup:

    close(fds[CHILD][WRITE]);
    close(fds[PARENT][READ]);

    destroy_buffer(&input_buf);

    free(args);

    return buf_out;
}
