#pragma once

#include "../../communication/buffer.h"
#include "../../functions/functions.h"
#include "../../result.h"

#include <fcntl.h>
#include <unistd.h>

/**
 * Put file command.
 *
 * Writes a file in a specified path with given content.
 *
 * @param[out] res  The result status of the command.
 * @param[in] buf   The buffer containing the parameters for the command:
 *                      1. unsigned int - The length of the path of the file to write.
 *                      2. char* - The path of the file to write.
 *                      3. char* - The content to write into the file.
 * @return  Buffer containing the results of the command (no results for this command).
 */
buffer put_file(result *res, buffer *buf);
