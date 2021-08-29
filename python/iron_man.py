import errno
import os
import select
import socket
import struct

import enum
import sys
from typing import List

import threading

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

    BUFFER_READING_OVERFLOW = 101,
    BUFFER_WRITING_OVERFLOW = 102,
    HANDSHAKE_FAILED = 103,
    UNKNOWN_COMMAND = 104,


class IronMan:
    HOST = '127.0.0.1'
    PORT = 25565
    KEY = [0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
           0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
           0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
           0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4]
    IV = [0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
          0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff]

    def __init__(self):
        self.ctx = AES_init_ctx_iv(self.KEY, self.IV)
        self.connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connection.connect((self.HOST, self.PORT))
        self.send('Q', 0x424021fca0537ac5)

    def __del__(self):
        self.send('Q', 0)
        self.connection.close()

    @staticmethod
    def to_bytes(string: str, null_terminator: bool = False) -> bytes:
        return bytes([ord(c) for c in string] + ([0] if null_terminator else []))

    def send(self, fmt: str, *args):
        message = struct.pack('<' + fmt, *args)
        size = struct.pack('<Q', len(message))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, size, len(size))]))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, message, len(message))]))

    def receive(self, fmt: str = None) -> str:
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
                f'Got failed result: {ResultCode((code,)).name} [{errno.errorcode[errno_value]} - {os.strerror(errno_value)}]')

    def get_file(self, path: str):
        self.send(f'Q{len(path) + 1}s', 0xdc3038f0f5c62a24, self.to_bytes(path, True))
        self.check_result()
        return self.receive()

    def put_file(self, path: str, content: str):
        self.send(f'QI{len(path) + 1}s{len(content)}s', 0xe02e89ab86f0651f, len(path) + 1,
                  self.to_bytes(path, True), self.to_bytes(content, True))
        self.check_result()
        self.receive()

    def run_shell(self, command: str, args: List[str] = None):
        args = args or []
        func_args = []
        for arg in args:
            func_args += [len(arg) + 1, self.to_bytes(arg, True)]

        self.send(f'QI{len(command) + 1}sI' + ''.join(f'I{len(arg) + 1}s' for arg in args),
                  0x2385d0791aec41e3, len(command) + 1, self.to_bytes(command, True), len(args), *func_args)

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


def main():
    iron_man = IronMan()

    iron_man.run_shell('wot')
    iron_man.run_shell('sleep', ['1'])
    iron_man.run_shell('sh')
    data = iron_man.get_file('/c/projects/iron-man/c/main.c')
    iron_man.put_file('/c/projects/iron-man/c/main.u', data)
    iron_man.get_file('/c/projects/iron-man/python/main.py')
    try:
        iron_man.get_file('/root')
        iron_man.get_file('/c/projects/iron-man/python/main.pyyy')
    except Exception as e:
        print(e)
    print(iron_man.get_file('/c/projects/iron-man/python/main.py'))


if __name__ == '__main__':
    main()
