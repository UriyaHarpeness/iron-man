#pragma once

#include "../../communication/buffer.h"
#include "../../result.h"

#include <sys/stat.h>

#define GET_FILE_COMMAND_ID 0xdc3038f0f5c62a24

buffer get_file(result *res, buffer *buf);
