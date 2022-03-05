#pragma once

#include "../../communication/buffer.h"
#include "../../functions/functions.h"
#include "../../result.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Get file command.
 *
 * Reads a file in a specified path.
 *
 * @param[out] res  The result status of the command.
 * @param[in] buf   The buffer containing the parameters for the command:
 *                      1. char* - The path of the file to get.
 * @return  Buffer containing the results of the command:
 *              1.  char* - The content of the file.
 */
buffer get_file(result *res, buffer *buf);
