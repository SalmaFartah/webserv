import socket
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8080))

# send all 3 requests at once without waiting
requests = (
    # "GET / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nworld\r\n6\r\n World\r\n1\r\n!\r\n0\r\n\r\n"
    "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "POST /webserv/timeout.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklnbyekkljnbyekkljj!!"
    # "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
)

s.sendall(requests.encode())
# time.sleep(15)

while True:
    data = s.recv(4096)
    if not data:
        break
    print("--- RECEIVED ---")
    print(data.decode())
s.close()

# time.sleep(60)
