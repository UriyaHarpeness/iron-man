#pragma once

#include "buffer.h"
#include "../commands/commands.h"
#include "../consts.h"
#include "../functions/functions.h"
#include "../logging/logging.h"
#include "../result.h"
#include "../tiny-aes/aes.h"

#include <netdb.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern int connection_fd;

void read_into_buffer(result *res, buffer *buf);

buffer read_buffer(result *res);

result send_string(char *string, uint64_t size);

result send_buffer(buffer buf);

result send_result(result res);

result connect_();

result communicate(const char *self_path);

void disconnect();
