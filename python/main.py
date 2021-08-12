import socket

HOST = '127.0.0.1'
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    data = None
    while data != b'exit':
        s.send(input().encode('utf-8'))
        data = s.recv(1024)
        print('Received', repr(data))
