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

from typing import Any, List

from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


class ResultCode(enum.Enum):
    SUCCESS = 0,

    FAILED_SOCKET = 1,
    FAILED_BIND = 2,
    FAILED_LISTEN = 3,
    FAILED_ACCEPT = 4,
    FAILED_READ = 5,
    FAILED_WRITE = 6,
    FAILED_MALLOC = 7,
    FAILED_STAT = 8,
    FAILED_OPEN = 9,
    FAILED_PIPE = 10,
    FAILED_FORK = 11,
    FAILED_DUP2 = 12,
    FAILED_EXECVP = 13,
    FAILED_SELECT = 14,
    FAILED_KILL = 15,
    FAILED_SYSCONF = 16,
    FAILED_MPROTECT = 17,
    FAILED_DLOPEN = 18,
    FAILED_DLSYM = 19,

    BUFFER_READING_OVERFLOW = 101,
    BUFFER_WRITING_OVERFLOW = 102,
    HANDSHAKE_FAILED = 103,
    UNKNOWN_COMMAND = 104,


class IronMan:
    HOST = '127.0.0.1'
    PORT = 25565

    def __init__(self, config_path: pathlib.Path):
        with config_path.open() as config:
            self.config = json.load(config)
        self.module_commands = {}
        self.ctx = AES_init_ctx_iv(self.config['generated_consts']['communication_key'],
                                   self.config['generated_consts']['communication_iv'])
        self.connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connection.connect((self.HOST, self.PORT))
        self.send('Q', self.config['generated_consts']['handshake'][0])

    def __del__(self):
        self.send('Q', 0)
        self.connection.close()

    @staticmethod
    def to_bytes(string: str, null_terminator: bool = False) -> bytes:
        return bytes([ord(c) for c in string] + ([0] if null_terminator else []))

    def send(self, fmt: str = None, *args: Any):
        if fmt is None:
            fmt = ''
        message = struct.pack('<' + fmt, *args)
        size = struct.pack('<Q', len(message))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, size, len(size))]))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, message, len(message))]))

    def receive(self, fmt: str = None) -> Any:
        size = self.connection.recv(8)
        size = struct.unpack('<Q', bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, size, len(size))]))[0]
        message = self.connection.recv(size)
        message = AES_CTR_xcrypt_buffer(self.ctx, message, len(message))
        if fmt is not None:
            message = struct.unpack('<' + fmt, self.to_bytes(message))
        return message

    def check_result(self):
        result = self.connection.recv(8)
        code, errno_value = struct.unpack('<II', self.to_bytes(AES_CTR_xcrypt_buffer(self.ctx, result, len(result))))
        if code != 0:
            raise ValueError(
                f'Got failed result: {ResultCode((code,)).name}' + (
                    f'[{errno.errorcode[errno_value]} - {os.strerror(errno_value)}]' if errno_value != 0 else ''))

    def send_run_command(self, command_name: str, fmt: str, *args: Any):
        command_id = self.config['generated_consts'][command_name + '_command_id'][0]
        key = bytearray(self.config['commands'][command_name][0])
        iv = bytearray(self.config['commands'][command_name][1])

        self.send(f'Q{len(key)}s{len(iv)}s' + fmt, command_id, key, iv, *args)

    def send_run_module_command(self, command_name: str, fmt: str, *args: Any):
        command_config = self.module_commands[command_name]
        command_id = command_config['command_id']
        key = bytearray(command_config['config']['key'])
        iv = bytearray(command_config['config']['iv'])

        self.send(f'Q{len(key)}s{len(iv)}s' + fmt, command_id, key, iv, *args)

    def get_file(self, path: str):
        self.send_run_command('get_file',
                              f'{len(path) + 1}s',
                              self.to_bytes(path, True))
        self.check_result()
        return self.receive()

    def put_file(self, path: str, content: str):
        self.send_run_command('put_file',
                              f'I{len(path) + 1}s{len(content)}s',
                              len(path) + 1, self.to_bytes(path, True), self.to_bytes(content, False))
        self.check_result()
        self.receive()

    def run_shell(self, command: str, args: List[str] = None) -> int:
        args = args or []
        func_args = []
        for arg in args:
            func_args += [len(arg) + 1, self.to_bytes(arg, True)]

        self.send_run_command('run_shell',
                              f'I{len(command) + 1}sI' + ''.join(f'I{len(arg) + 1}s' for arg in args),
                              len(command) + 1, self.to_bytes(command, True), len(args), *func_args)

        stop = False
        while not stop:
            while not select.select([self.connection, sys.stdin], [], [], 0.05)[0]:
                pass

            for readable in select.select([self.connection, sys.stdin], [], [], 0.05)[0]:
                if readable is self.connection:
                    output = self.receive()
                    if len(output) == 0:
                        print('Child process died')
                        stop = True
                        continue

                    print('=' * 25, 'got', '=' * 25)
                    print(output)
                    print('-' * 55)

                elif readable is sys.stdin:
                    message = input()
                    print('>>>', message)
                    if message == 'KILL!':
                        print('Killing child process')
                        self.send('I', 0)
                        stop = True
                        continue

                    self.send(f'I{len(message) + 1}s', len(message) + 1, self.to_bytes(message, True))

        self.check_result()
        return self.receive('B')[0]

    def run_module_command(self, function_name: str, *args: Any, arguments_format: str = None,
                           results_format: str = None):
        if arguments_format is None:
            arguments_format = self.module_commands[function_name]['invocation']['arguments_format']
        if results_format is None:
            results_format = self.module_commands[function_name]['invocation']['results_format']
        self.send_run_module_command(function_name, arguments_format, *args)
        self.check_result()
        return self.receive(results_format)

    def add_module_command(self, module_path: pathlib.Path, function_name: str, module_config_path: pathlib.Path,
                           arguments_format: str = None, results_format: str = None):
        with module_config_path.open() as module_config:
            function_config = json.load(module_config)[function_name]

        command_id = random.randint(0, 2 ** 32 - 1)
        self.module_commands[function_name] = {'config': function_config, 'command_id': command_id,
                                               'invocation': {'arguments_format': arguments_format,
                                                              'results_format': results_format}}
        self.send(f'QI{len(str(module_path)) + 1}sI{len(function_name) + 1}sQQ',
                  1, len(str(module_path)) + 1, self.to_bytes(str(module_path), True),
                  len(function_name) + 1, self.to_bytes(function_name, True), function_config['size'], command_id)

        self.check_result()

        self.__setattr__(function_name, lambda *args, **kwargs: self.run_module_command(function_name, *args, **kwargs))

    def remove_module_command(self, function_name: str):
        self.send(f'QQ', 2, self.module_commands[function_name]['command_id'])

        self.check_result()

        self.module_commands.pop(function_name)
        self.__delattr__(function_name)

    # todo: add module commands support in iron_cmd


def main():
    iron_man = IronMan(pathlib.Path('iron_man_config.json'))

    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'sum',
        pathlib.Path('module_config.json'), 'II', 'I')
    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'difference',
        pathlib.Path('module_config.json'), 'II', 'I')
    print(iron_man.sum(4, 4)[0])
    print(iron_man.sum(4, 400)[0])
    print(iron_man.sum(1000, 24)[0])
    iron_man.remove_module_command('sum')
    print(iron_man.difference(4, 4)[0])
    print(iron_man.difference(4, 400)[0])
    print(iron_man.difference(1000, 24)[0])

    data = iron_man.get_file('../main.c')
    iron_man.run_shell('wot')
    iron_man.run_shell('sleep', ['1'])
    iron_man.run_shell('ls')
    iron_man.put_file('../main.c.copy', data)
    try:
        iron_man.get_file('/root')
        iron_man.get_file('../main.pyyy')
    except Exception as e:
        print(e)
    print(iron_man.get_file('../main.c'))


if __name__ == '__main__':
    main()
