# *This project has been created as part of the 42 curriculum by sfartah, ychadmi, kaneddam.*

# Webserv

## Description

**Webserv** is an HTTP/1.1 web server written entirely in **C++98**.

The purpose of this project is to understand how a web server works by implementing one from scratch without using external networking or HTTP libraries. The server relies on non-blocking sockets and an event-driven architecture to efficiently handle multiple client connections simultaneously.

The implementation supports configurable virtual servers through a configuration file and provides a subset of the features commonly found in modern web servers.

### Features

- HTTP/1.1 support
- Multiple virtual servers
- Multiple listening ports
- Non-blocking sockets using `poll()`
- GET, POST and DELETE methods
- Static file serving
- Autoindex
- Custom error pages
- File uploads
- CGI execution
- HTTP redirections
- Configurable locations
- Client body size limitation
- Persistent connections (Keep-Alive)

---

# Architecture

The project is organized into several independent modules.

### Server

Responsible for:

- socket creation
- listening for incoming connections
- client management
- event polling
- request dispatching

### Configuration Parser

Responsible for:

- parsing the configuration file
- validating directives
- building the server configuration

### HTTP Request

Responsible for:

- parsing the request line
- parsing headers
- parsing the body
- validating HTTP requests

### HTTP Response

Responsible for:

- generating HTTP responses
- serving static resources
- generating error pages
- handling redirections

### CGI

Responsible for:

- executing CGI scripts
- preparing environment variables
- reading CGI output
- forwarding CGI responses to the client

---

# Instructions

## Requirements

The project requires:

- Linux or macOS
- A C++98 compatible compiler (`g++` or `clang++`)
- GNU Make

Optional dependencies:

- Python 3 (Python CGI)
- PHP-CGI (PHP CGI)
- curl (testing)

## Compilation

Compile the project:

```bash
make
```

Clean object files:

```bash
make clean
```

Remove all generated files:

```bash
make fclean
```

Rebuild everything:

```bash
make re
```

---

# Usage

Launch the server with a configuration file:

```bash
./webserv config/default.conf
```

or simply

```bash
./webserv
```

Then open:

```
http://localhost:8080
```

Example request:

```bash
curl http://localhost:8080
```

---

# Example Configuration

```conf
server
{
    listen 8080;
    root ./www;
    index index.html;

    location /
    {
        autoindex on;
    }

    location /cgi
    {
        cgi_extension .py;
        cgi_pass /usr/bin/python3;
    }

    location /upload
    {
        upload_store ./uploads;
    }
}
```

---

# Supported HTTP Methods

- GET
- POST
- DELETE

---

# Supported Status Codes

- 200 OK
- 201 Created
- 204 No Content
- 301 Moved Permanently
- 400 Bad Request
- 403 Forbidden
- 404 Not Found
- 405 Method Not Allowed
- 413 Payload Too Large
- 500 Internal Server Error
- 501 Not Implemented
- 505 HTTP Version Not Supported

---

# Testing

Useful tools:

```bash
curl
```

```bash
nc
```

```bash
telnet
```

```bash
ab
```

```bash
siege
```

---

# Technologies

- C++98
- POSIX
- BSD Sockets API
- poll()
- HTTP/1.1
- CGI

---

# Resources

### HTTP

- RFC 7230 – HTTP/1.1 Message Syntax and Routing
- RFC 7231 – HTTP/1.1 Semantics and Content
- MDN HTTP Documentation

### CGI

- RFC 3875
- CGI Specification

### Socket Programming

- Beej's Guide to Network Programming

### Reference

- NGINX Documentation
- cppreference.com

---

# AI Usage

Artificial Intelligence (ChatGPT) was used exclusively as a learning and documentation assistant.

It was used to:

- clarify HTTP concepts and RFC specifications;
- improve the structure and wording of the documentation;
- suggest test scenarios using `curl`;
- provide ideas for stress testing and CGI validation.

All project architecture, implementation, algorithms and source code were designed and written by the project authors.

---

# Authors

- **sfartah**
- **ychadmi**
- **kaneddam**

42 School