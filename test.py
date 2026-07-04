import socket
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8080))

# send all 3 requests at once without waiting
requests = (
    # "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n World\r\n1\r\n!\r\n0\r\n\r\n"
    # "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "POST /upload HTTP/1.1\r\nHost: localhost:3000\r\nContent-Type: multipart/form-data; boundary=----MyBoundary123456\r\nContent-Length: 449\r\n\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"files\"; filename=\"photo1.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n<binary photo1>\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"files\"; filename=\"photo2.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n<binary photo2>\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"files\"; filename=\"document.pdf\"\r\nContent-Type: application/pdf\r\n\r\n<binary document>\r\n------MyBoundary123456--\r\n"
    # "POST /webserv/test.sh HTTP/1.1\r\nHost: localhost\r\nContent-Length: 2000\r\n\r\nLorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit cursus nunc, quis gravida magna mi a libero. Fusce vulputate eleifend sapien. Vestibulum purus quam, scelerisque ut, mollis sed, nonummy id, metus. Nullam accumsan lorem in dui. Cras ultricies mi eu turpis hendrerit fringilla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In ac dui quis mi consectetuer lacinia. Nam pretium turpis et arcu. Duis arcu tortor, suscipit eget, imperdiet nec, imperdiet iaculis, ipsum. Sed aliquam ultrices mauris. Integer ante arcu, accumsan a, consectetuer eget, posuere ut, mauris. Praesent adipiscing. Phasellus ullamcorper ipsum rutrum nunc. Nunc nonummy metus. Vestib"
    # "POST /webserv/timeout.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET /webserv/testPyth.py HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 9\r\n\r\nbyekklj!!"
    # "POST /upload HTTP/1.1\r\nHost: localhost:3000\r\nContent-Type: multipart/form-data; boundary=----MyBoundary123456\r\nContent-Length: 153\r\n\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=file; filename=test.txt\r\nContent-Type: text/plain\r\n\r\nhello world\r\n------MyBoundary123456--\r\n"
    "POST /upload HTTP/1.1\r\nHost: localhost:3000\r\nConnection: close\r\nContent-Type: multipart/form-data; boundary=----MyBoundary123456\r\nContent-Length: 546\r\n\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"field1\"; filename=\"field1.txt\"\r\nContent-Type: text/plain\r\n\r\nvalue1\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"field2\"; filename=\"field2.txt\"\r\nContent-Type: text/plain\r\n\r\nvalue2\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"field3\"; filename=\"field3.txt\"\r\nContent-Type: text/plain\r\n\r\nvalue3\r\n------MyBoundary123456\r\nContent-Disposition: form-data; name=\"field4\"; filename=\"field4.txt\"\r\nContent-Type: text/plain\r\n\r\nvalue4\r\n------MyBoundary123456--\r\n"
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
