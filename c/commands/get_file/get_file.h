#pragma once

#include "../../communication/buffer.h"
#include "../../result.h"

#include <sys/stat.h>

buffer get_file(result *res, buffer *buf);
