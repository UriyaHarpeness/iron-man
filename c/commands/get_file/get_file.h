#pragma once

#include "../../communication/buffer.h"
#include "../../result.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

buffer get_file(result *res, buffer *buf);
