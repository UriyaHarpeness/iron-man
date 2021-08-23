#pragma once

#include "../../communication/buffer.h"
#include "../../result.h"

#define PUT_FILE_COMMAND_ID 0xe02e89ab86f0651f

buffer put_file(result *res, buffer *buf);
