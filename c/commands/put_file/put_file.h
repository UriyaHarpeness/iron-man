#pragma once

#include "../../communication/buffer.h"
#include "../../result.h"

#include <fcntl.h>
#include <unistd.h>

buffer put_file(result *res, buffer *buf);
