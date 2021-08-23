#pragma once

#include "../communication/buffer.h"
#include "get_file/get_file.h"
#include "put_file/put_file.h"
#include "../result.h"

buffer run_command(result *res, uint64_t command_id, buffer *buf);
