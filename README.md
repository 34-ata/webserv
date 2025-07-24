# WebServ - HTTP/1.1 Web Server

A high-performance, multi-server HTTP/1.1 web server implementation written in C++. This project supports multiple virtual hosts, CGI execution, file uploads, and various HTTP methods with a flexible configuration system.

## 🚀 Features

### Core HTTP Support
- **HTTP/1.1 Protocol**: Full compliance with HTTP/1.1 specifications
- **Multiple HTTP Methods**: GET, POST, DELETE support
- **Keep-Alive Connections**: Persistent connection support for better performance
- **Custom Error Pages**: Configurable error pages for different HTTP status codes

### Advanced Server Capabilities
- **Virtual Hosts**: Multiple server configurations on different ports
- **CGI Support**: Execute Python scripts (.py files) via CGI interface
- **File Upload**: Handle file uploads with configurable storage locations
- **Directory Listing**: Automatic directory indexing when enabled
- **URL Redirection**: Support for 301/302 redirects

### Performance & Reliability
- **Non-blocking I/O**: Asynchronous handling using `poll()` system call
- **Connection Timeouts**: Automatic cleanup of idle connections (15s timeout)
- **Request Size Limits**: Configurable maximum request body size
- **Memory Management**: Proper resource cleanup and memory management

## 📋 Requirements

- **C++98 Compatible Compiler** (g++, clang++)
- **Unix-like Operating System** (Linux, macOS)
- **Python 3** (for CGI script execution)
- **Standard POSIX Libraries**

## 🛠️ Installation

1. **Clone the repository:**
```bash
git clone https://github.com/yourusername/webserv.git
cd webserv
```

2. **Compile the project:**
```bash
make
```

3. **Run the server:**
```bash
./webserv [config_file]
```

If no config file is provided, it defaults to `./config`.

## ⚙️ Configuration

The server uses a nginx-like configuration syntax. Here's the structure:

### Basic Server Block
```nginx
server {
    listen 8080;                    # Port to listen on
    server_name localhost;          # Server hostname
    root /www;                      # Document root
    client_max_body_size 1000000;   # Max request body size in bytes
    
    # Error page configurations
    error_page 404 /www/errors/404.html;
    error_page 500 /www/errors/500.html;
    
    # Location blocks for specific routes
    location / {
        root /www;
        index index.html;
        methods GET POST DELETE;    # Allowed HTTP methods
        autoindex off;              # Directory listing
    }
}
```

### Location Block Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| `root` | Document root for this location | `root /var/www/html;` |
| `index` | Default index file | `index index.html;` |
| `methods` | Allowed HTTP methods | `methods GET POST;` |
| `autoindex` | Enable/disable directory listing | `autoindex on;` |
| `return` | HTTP redirect | `return 301 /new-page.html;` |
| `cgi_extension` | CGI file extension | `cgi_extension .py;` |
| `cgi_path` | CGI interpreter path | `cgi_path /usr/bin/python3;` |
| `upload_store` | File upload directory | `upload_store ./uploads;` |

### Multi-Server Configuration Example
```nginx
# Main Web Server
server {
    listen 8080;
    server_name localhost;
    root /www;
    
    location / {
        methods GET POST DELETE;
        index index.html;
    }
    
    location /cgi-bin {
        root /www/cgi-bin;
        methods GET POST;
        cgi_extension .py;
        cgi_path /usr/bin/python3;
    }
    
    location /uploads {
        root /www/uploads;
        methods GET POST DELETE;
        autoindex on;
        upload_store ./www/uploads;
    }
}

# API Server
server {
    listen 8081;
    server_name api.localhost;
    root /api;
    client_max_body_size 500000;
    
    location / {
        methods GET POST PUT DELETE;
    }
}
```

## 🔧 Usage Examples

### Starting the Server
```bash
# Use default config
./webserv

# Use custom config
./webserv my_config.conf
```

### Testing with curl

**GET Request:**
```bash
curl -X GET http://localhost:8080/
```

**POST Request (File Upload):**
```bash
curl -X POST -d "Hello World" http://localhost:8080/uploads/
```

**DELETE Request:**
```bash
curl -X DELETE http://localhost:8080/uploads/somefile.txt
```

**CGI Script Execution:**
```bash
curl -X GET http://localhost:8080/cgi-bin/script.py
curl -X POST -d "param=value" http://localhost:8080/cgi-bin/form.py
```

## 📁 Project Structure

```
webserv/
├── includes/           # Header files
│   ├── HttpMethods.hpp
│   ├── Request.hpp
│   ├── Response.hpp
│   ├── Server.hpp
│   ├── WebServer.hpp
│   └── ...
├── src/               # Source files
│   ├── main.cpp
│   ├── WebServer.cpp
│   ├── Server.cpp
│   ├── Request.cpp
│   ├── Response.cpp
│   └── ...
├── config             # Default configuration
├── Makefile
└── README.md
```

## 🏗️ Architecture

### Class Overview

- **WebServer**: Main server orchestrator, handles configuration parsing and server management
- **Server**: Individual server instance, manages connections and request processing
- **Request**: HTTP request parser and validator
- **Response**: HTTP response builder
- **Tokenizer**: Configuration file lexical analyzer

### Request Processing Flow

1. **Connection Accept**: New connections accepted on listening sockets
2. **Request Parsing**: HTTP request headers and body parsed
3. **Route Matching**: Request URI matched against location blocks
4. **Method Validation**: HTTP method checked against allowed methods
5. **Content Processing**: 
   - Static files served directly
   - CGI scripts executed
   - File uploads handled
6. **Response Generation**: HTTP response built and sent
7. **Connection Management**: Keep-alive or connection close

## 🧪 Testing

### Manual Testing
Create test files in your document root:
```bash
mkdir -p www/cgi-bin www/uploads
echo "<h1>Hello World</h1>" > www/index.html
echo "print('Content-Type: text/html\n\n<h1>CGI Works!</h1>')" > www/cgi-bin/test.py
chmod +x www/cgi-bin/test.py
```

### Load Testing
```bash
# Using ab (Apache Bench)
ab -n 1000 -c 10 http://localhost:8080/

# Using curl for stress testing
for i in {1..100}; do curl http://localhost:8080/ & done
```

## 🚨 Error Handling

The server handles various error conditions:

- **400 Bad Request**: Malformed HTTP requests
- **403 Forbidden**: Access denied or method not allowed
- **404 Not Found**: Resource not found
- **405 Method Not Allowed**: HTTP method not permitted
- **413 Payload Too Large**: Request body exceeds limit
- **500 Internal Server Error**: Server-side errors

## ⚡ Performance Features

- **Non-blocking I/O**: Uses `poll()` for efficient connection handling
- **Connection Pooling**: Reuses connections with HTTP/1.1 keep-alive
- **Memory Efficient**: Minimal memory footprint per connection
- **Timeout Management**: Automatic cleanup of stale connections

## 🔒 Security Considerations

- **Path Traversal Protection**: Prevents access outside document root
- **Request Size Limits**: Protects against DoS attacks
- **Method Restrictions**: Configurable HTTP method permissions
- **CGI Security**: Controlled script execution environment

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🏆 Acknowledgments

- Inspired by nginx configuration syntax
- Built as part of 42 School curriculum
- HTTP/1.1 specification compliance

## 📞 Support

For issues and questions:
- Open an issue on GitHub
- Check existing documentation
- Review configuration examples

---

**Made with ❤️ in C++**
