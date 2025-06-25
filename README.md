![Webserv](https://img.shields.io/badge/Webserv-HTTP%20Server-blue?style=for-the-badge&logo=appveyor)
![C++](https://img.shields.io/badge/C++-98-blue?style=for-the-badge&logo=appveyor)
![HTTP](https://img.shields.io/badge/HTTP-1.1-blue?style=for-the-badge&logo=appveyor)
# webserv
```
FINAL GRADE: ---/100
```
**Made by:** **[@kyeh](https://github.com/kyeh)**, **[@rogalio](https://github.com/rogalio)**, **[@oprosvir](https://github.com/oprosvir)**  

## 📖 Project Description
**Webserv** is a fully functional HTTP server written in **C++98**, designed to deepen understanding of network programming and the HTTP protocol. This project involves implementing essential web server functionality from scratch, including handling connections, parsing HTTP requests, generating responses, and supporting configuration-based behavior.

The server supports the **GET**, **POST**, and **DELETE** methods, serves static websites, allows file uploads, and is compatible with modern browsers. Configuration is done through a JSON-based file, inspired by the structure of NGINX, allowing specification of ports, hosts, server names, error pages, client body size limits, and route-specific behaviors (e.g., allowed methods, redirection, directory listing, CGI execution).

## 🚀 Features
- Non-blocking I/O using a single `epoll()`
- Support for multiple ports and virtual servers
- Default error handling and file serving
- Integration with CGI scripts
- Stress-resilience: the server must remain available under heavy load

### ✨ Bonus Features
- ✅Cookie and session handling
- ✅Multiple CGI support

### Configuration Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| 🔧 **Server-level directives** |  |  |
| `name` | Server name (optional) | `"name": "main server"` |
| `host` | Address to bind the server to | `"host": "0.0.0.0"` |
| `port` | Port to listen on | `"port": 8080` |
| `max_body` | Request body limit | `"max_body": 1000000` |
| `error` | Custom error pages | `"error": { "404": "./error/404.html" }` |
| `routes` | Array of route definitions | `"routes": [ { ... }, { ... } ]` |
| 🛣 **Route-level directives** | | |
| `route` | Path to match in the URL | `"route": "/about"` |
| `method` | Allowed HTTP methods | `"methods": ["GET", "POST"]` |
| `directory` | Physical path to serve files from | `"directory": "./website"` |
| `index` | Default file to serve if the request points to a directory | `"index": "index.html"` |
| `dir_listing` | Enable or disable directory listing | `"dir_listing": true` |
| `upload` | Directory where uploaded files should be stored | `"upload": "./upload"` |
| `redirection` | URL to redirect requests to | `"redirection": "https://example.com"` |
| `cgi` | List of CGI interpreters for specific extensions | `"cgi": [ { "extension": "py", "exec": "/usr/bin/python3" } ] ` |

## 📦 Installation

- **System**: Linux

```bash
git clone https://github.com/42-mates/webserv.git && cd webserv && make
```
```bash
# Run with default configuration
./webserv

# Or specify a custom config file
./webserv config/redirection.json
```
