# Webserv

A lightweight HTTP/1.1 web server built from scratch in **C++98** as part of the 42 curriculum.

Webserv is designed to make the fundamentals of web-server engineering visible: socket management, HTTP message parsing, request routing, response generation, configuration handling, file serving, uploads, and CGI execution. It uses non-blocking sockets and an event-driven loop based on `poll()` to serve multiple clients without creating one thread per connection.

## What the project can do

- Serve static files and directories over HTTP/1.1.
- Handle `GET`, `POST`, and `DELETE` requests.
- Listen on one or more configured ports and select the appropriate virtual-server configuration.
- Route requests through configurable `location` blocks.
- Generate directory listings with `autoindex`.
- Apply custom error responses for common HTTP failures.
- Enforce request-body limits and return `413 Payload Too Large` when required.
- Redirect directory requests when the target path is missing its trailing slash.
- Accept file uploads and store them in a configured directory.
- Execute CGI programs for configured file extensions and forward their output to the client.
- Preserve HTTP connection behavior such as `Connection: keep-alive`.

## Main concepts

### Event-driven networking

The server creates listening sockets, accepts clients, and monitors client and CGI file descriptors with `poll()`. Readable and writable events are processed without blocking the other connections handled by the server.

### Configuration parsing

A configuration file is tokenized and parsed into server and location settings. These settings control listening ports, document roots, allowed methods, indexes, autoindexing, CGI behavior, uploads, redirects, and request-size limits.

### HTTP request parsing

Incoming bytes are converted into a structured request containing the method, target, HTTP version, headers, body, connection state, and upload data. Invalid request lines, headers, methods, versions, or body sizes are rejected with an appropriate error response.

### Routing and response generation

The router chooses the best matching location, resolves the requested filesystem path, checks permissions and allowed methods, and dispatches the request to static-file handling, directory handling, upload handling, deletion, redirection, or CGI execution. The response builder then creates a complete HTTP response with status line, headers, content length, and body.

### CGI execution

CGI requests are executed through child processes and pipes. The server prepares the CGI environment, writes the request body to the child process, reads the generated output, and converts it into an HTTP response while continuing to monitor the related descriptors through the event loop.

## Project structure

```text
.
├── main.cpp                    # Entry point and server startup
├── event_loop/                 # poll()-based connection and event management
├── parse_config/               # Configuration tokenization and parsing
├── parse_request/              # HTTP request parsing and validation
├── build_response/             # HTTP response and error-page generation
├── route/                      # Request routing and resource dispatch
├── uploadCgi/                  # Upload handling and CGI execution
├── conf_file_example/          # Example configuration files
├── nginx.conf                  # Default configuration used by the executable
└── Makefile                    # C++98 build rules
```

## Requirements

- A POSIX-compatible environment such as Linux or macOS
- A C++ compiler with C++98 support
- GNU Make
- `curl` for basic manual testing
- Python or another CGI runtime if CGI scripts are used

## Build

```bash
make
```

Other useful targets:

```bash
make clean    # Remove object files
make fclean   # Remove object files and the executable
make re       # Rebuild from scratch
make ac       # Build, then remove object files
```

## Run the server

Run with the default `nginx.conf` configuration:

```bash
./webserv
```

Or provide another configuration file:

```bash
./webserv path/to/config.conf
```

With the default configuration, the server listens on port `8080`:

```bash
curl -i http://localhost:8080/
```

You can also open `http://localhost:8080` in a browser.

## Configuration example

```nginx
server
{
    listen 8080;
    root /path/to/webserv;

    location /
    {
        autoindex on;
        allowed_method GET POST;
        # cgi_pass /usr/bin/python3;
        # cgi_extension .py;
        # upload_store /path/to/uploads;
    }
}
```

The repository also contains `conf_file_example/test.conf` as an additional configuration reference. Paths are environment-specific and should be updated before running the server.

Common configuration concepts include:

| Directive | Purpose |
| --- | --- |
| `listen` | Select the port on which a server listens. |
| `root` | Set the filesystem root for requested resources. |
| `location` | Define routing and behavior for a URL prefix. |
| `allowed_method` | Restrict the HTTP methods accepted by a location. |
| `autoindex` | Enable directory listings. |
| `index` | Define the default file used for a directory request. |
| `cgi_pass` / `cgi_extension` | Enable CGI execution for matching files. |
| `upload_store` | Select where uploaded files are stored. |

## HTTP behavior

### Methods

- `GET` — retrieve files, directories, generated listings, or CGI output.
- `POST` — submit request bodies, upload files, or invoke CGI programs.
- `DELETE` — remove a resource when the configured path and permissions allow it.

### Status codes handled

The response layer includes common success, redirection, client-error, and server-error responses, including:

`200 OK`, `201 Created`, `204 No Content`, `301 Moved Permanently`, `400 Bad Request`, `403 Forbidden`, `404 Not Found`, `405 Method Not Allowed`, `413 Payload Too Large`, and `500 Internal Server Error`.

## Testing ideas

Use `curl` to exercise the main server paths:

```bash
curl -i http://localhost:8080/
curl -i -X POST -d 'hello=webserv' http://localhost:8080/
curl -i -X DELETE http://localhost:8080/file.txt
curl -i http://localhost:8080/nonexistent
```

For lower-level and load-oriented checks, tools such as `nc`, `telnet`, `ab`, or `siege` can be useful. Test both valid and malformed requests, unsupported methods, missing resources, directory access, body-size limits, uploads, and CGI scripts.

## What I learned

- How TCP listening sockets, client sockets, file descriptors, and non-blocking I/O work together.
- How an event loop with `poll()` can coordinate many clients and CGI pipes without blocking the whole server.
- How an HTTP/1.1 message is structured and why request-line, header, body, and connection rules must be validated separately.
- How URL paths are mapped safely to filesystem resources and then dispatched according to the selected location.
- How status codes and response headers communicate the result of a request to a client.
- How CGI processes communicate with a server through environment variables, pipes, and process management.
- How configuration parsing turns a text file into validated runtime behavior.
- How to test a network service from both a browser and command-line clients.

### My contribution

My main contribution to this project was the HTTP message layer:

- **Parsing HTTP requests:** extracting and validating the request line, headers, body, HTTP version, method, and request state.
- **Building HTTP responses:** generating status lines, headers, content length, response bodies, redirections, and error responses for the different request outcomes.

## Resources

- [RFC 7230 — HTTP/1.1 Message Syntax and Routing](https://www.rfc-editor.org/rfc/rfc7230)
- [RFC 7231 — HTTP/1.1 Semantics and Content](https://www.rfc-editor.org/rfc/rfc7231)
- [RFC 3875 — The Common Gateway Interface (CGI)](https://www.rfc-editor.org/rfc/rfc3875)
- [MDN — HTTP Overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [C++ reference](https://en.cppreference.com/w/cpp)
- [NGINX documentation](https://nginx.org/en/docs/)

42 School project.
