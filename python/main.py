import socket
import struct

from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


class IronMan:
    HOST = '127.0.0.1'
    PORT = 8080
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
        self.connection.close()

    def send(self, fmt: str, *args):
        message = struct.pack(fmt, *args)
        size = struct.pack('Q', len(message))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, size, len(size))]))
        self.connection.send(bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, message, len(message))]))

    def receive(self):
        size = self.connection.recv(8)
        size = struct.unpack('Q', bytes([ord(c) for c in AES_CTR_xcrypt_buffer(self.ctx, size, len(size))]))[0]
        message = self.connection.recv(size)
        message = AES_CTR_xcrypt_buffer(self.ctx, message, len(message))
        return message


iron_man = IronMan()

MSGS = ['good', 'too', 'exit']
for msg in MSGS:
    print('Sending', msg)
    iron_man.send("%ds" % (len(msg),), bytes([ord(c) for c in msg]))
    got = iron_man.receive()
    print('Received', got)
