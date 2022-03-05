#pragma once

#include "../../communication/buffer.h"
#include "../../functions/functions.h"
#include "../../result.h"

/**
 * Sum command.
 *
 * Calculates the sum of 2 numbers.
 *
 * @param[out] res  The result status of the command.
 * @param[in] buf   The buffer containing the parameters for the command:
 *                      1. unsigned int - The first number.
 *                      2. unsigned int - The second number.
 * @return  Buffer containing the results of the command:
 *              1.  unsigned int - The sum of the 2 numbers.
 */
buffer sum(result *res, buffer *buf);
