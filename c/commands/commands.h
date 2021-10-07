#pragma once

#include "../communication/buffer.h"
#include "get_file/get_file.h"
#include "put_file/put_file.h"
#include "run_shell/run_shell.h"
#include "../result.h"
#include "../strings/strings.h"

result initialize_commands();

void add_command(result *res, const char *module_path);

void destroy_commands();

result xcrypt_command(const char *key, const char *iv, const unsigned char *start_address,
                      const unsigned char *end_address);

buffer run_command(result *res, uint64_t command_id, const char *key, const char *iv, buffer *buf);
