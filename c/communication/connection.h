#pragma once

#include "buffer.h"
#include "../commands/commands.h"
#include "../consts.h"
#include "../logging/logging.h"
#include "../result.h"
#include "../tiny-aes/aes.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

uint64_t char_to_uint64(const char buf[8]);

void uint64_to_char(uint64_t value, char buf[8]);

void int_to_char(int value, char buf[4]);

uint64_t read_uint64_t(result *res, buffer *buf);

void read_into_buffer(result *res, buffer *buf);

buffer read_buffer(result *res);

result send_buffer(buffer buf);

result send_result(result res);

result connect_();

result communicate();

void disconnect();
