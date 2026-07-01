#!/usr/bin/env python3
import os, sys

length = int(os.environ.get("CONTENT_LENGTH", 0))

body = sys.stdin.read(length) if length > 0 else ""

print("Content-Type: text/html\r")
print("\r")
print("<h1>POST received</h1>")
print("<p>Body: " + body + "</p>")

while True:
    pass