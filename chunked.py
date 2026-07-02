import socket

def make_chunked_body(data, chunk_size):
    result = ""
    for i in range(0, len(data), chunk_size):
        chunk = data[i:i + chunk_size]
        result += hex(len(chunk))[2:] + "\r\n"
        result += chunk + "\r\n"
    result += "0\r\n\r\n"
    return result

# generate large body
body_data = "A" * 100000  # 100KB of data
chunked_body = make_chunked_body(body_data, 1024)  # 1KB chunks

request = (
    "POST /upload HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "connection: keep-alive\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    + chunked_body
)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 8080))
s.sendall(request.encode())

response = s.recv(65536)
print(response.decode())
s.close()