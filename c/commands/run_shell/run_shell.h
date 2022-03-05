#pragma once

#include "../../communication/buffer.h"
#include "../../communication/connection.h"
#include "../../functions/functions.h"
#include "../../result.h"

#include <signal.h>
#include <sys/wait.h>

/**
 * Run shell command.
 *
 * Runs a command with given arguments.
 *
 * @param[out] res  The result status of the command.
 * @param[in] buf   The buffer containing the parameters for the command:
 *                      1. unsigned int - The length of the command to execute.
 *                      2. char* - The command to execute.
 *                      3. unsigned int - The number of arguments for the command.
 *                      4. (for each argument):
 *                          1.  unsigned int - The length of the argument.
 *                          2.  char* - The argument.
 * @return  Buffer containing the results of the command:
 *              1.  uint8_t - The command's exit status.
 */
buffer run_shell(result *res, buffer *buf);
