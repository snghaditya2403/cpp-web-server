# HTTP Server in C++

A multi-threaded HTTP/1.1 web server built from scratch in C++ using POSIX sockets.

## Features
- Handles concurrent connections using `std::thread`
- Serves static files (HTML, CSS, JS, JPEG)
- Automatic MIME type detection
- Returns proper 404 responses for missing files
- Defaults to `index.html` for root path `/`

## How to Build and Run
```bash
g++ -std=c++17 server.cpp -o server -pthread
./server
```
Then open your browser and go to `http://localhost:8080`.

## Tech Used
- C++17
- POSIX Sockets
- `std::thread` for concurrency
- HTTP/1.1 protocol
