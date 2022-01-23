#pragma once

#include "../communication/buffer.h"
#include "get_file/get_file.h"
#include "put_file/put_file.h"
#include "run_shell/run_shell.h"
#include "../result.h"
#include "../strings/strings.h"

result initialize_commands();

void add_module_command(result *res, const char *module_path, const char *function_name, uint64_t function_size,
                        uint64_t command_id);

void add_module_command_from_buffer(result *res, buffer *buf);

result xcrypt_command(const char *key, const char *iv, const unsigned char *start_address,
                      const unsigned char *end_address);

buffer run_command(result *res, uint64_t command_id, const char *key, const char *iv, buffer *buf);

void remove_module_command(result *res, uint64_t command_id);

void remove_module_command_from_buffer(result *res, buffer *buf);

void destroy_module_commands();

void destroy_commands();
