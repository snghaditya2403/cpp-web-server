#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <fstream>

void handle_client(int client_fd){
	char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer));

	//String conversion of buffer
	std::string request_str(buffer);
	std::stringstream ss(request_str);

	std::string method, path, protocol;
	ss >> method;
	ss >> path;
	ss >> protocol;

// ... (Previous code: reading the buffer and extracting the path using stringstream) ...

    std::cout << "[Worker Thread " << std::this_thread::get_id() << "] requested: " << path << "\n";

    // 1. Dynamic Pathing: Default to index.html if they just type "/", otherwise use the path
    std::string file_path;
    if (path == "/") {
        file_path = "index.html";
    } else {
        // .substr(1) removes the leading '/' so "/style.css" becomes "style.css"
        file_path = path.substr(1); 
    }

    // 2. Determine the MIME Type based on the file extension
    std::string content_type = "text/plain"; // Default fallback
    if (file_path.find(".html") != std::string::npos) content_type = "text/html";
    else if (file_path.find(".css") != std::string::npos) content_type = "text/css";
    else if (file_path.find(".js") != std::string::npos) content_type = "application/javascript";
    else if (file_path.find(".jpg") != std::string::npos || file_path.find(".jpeg") != std::string::npos) content_type = "image/jpeg";

    // 3. Open the file in binary mode (required for images/compiled files)
    std::ifstream file(file_path, std::ios::binary);

    std::string http_response;

    if (file.is_open()) {
        // File exists! Read it and send 200 OK
        std::stringstream file_buffer;
        file_buffer << file.rdbuf();
        std::string body = file_buffer.str();
        
        http_response = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: " + content_type + "\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                        "\r\n" + 
                        body;
        file.close();
    } else {
        // File not found on the hard drive! Send 404
        std::string body = "<html><body><h1>404 Not Found</h1><p>The file " + file_path + " does not exist.</p></body></html>";
        http_response = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                        "\r\n" + 
                        body;
    }

    write(client_fd, http_response.c_str(), http_response.length());

    close(client_fd);

}

class HttpServer{
private:
	int port;                               //port we are using for the server
	int server_fd;                          //our server file descriptor for the socket                         
	struct sockaddr_in server_address;      //our server address

public:
	HttpServer(int server_port){
		port = server_port;
		server_fd = socket(AF_INET, SOCK_STREAM, 0);

		if(server_fd < 0){
			std::cerr << "Error: Failed to create the socket!\n";
			return;
		}

		//Using socket to internet
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(port);

		int bind_res = bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address));

		if(bind_res < 0){
			std::cerr << "Error: Failed to bind the Socket to Port " << port << '\n';
			return;
		}

		if(listen(server_fd, 10)){
			std::cerr << "Error: Failed to listen on port " << port << "!\n";
            return;
		}
	}

	void start() {
		std::cout << "Server is now waiting for a connection... (Code is paused here)\n";

		while(true){
			//Client address
			struct sockaddr_in client_address;
        	socklen_t client_address_len = sizeof(client_address);

        	int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_len);

        	if (client_fd < 0) {
            	std::cerr << "Error: Failed to accept the connection!\n";
            	return;
        	}

        	//Thread
        	std::thread worker(handle_client, client_fd);
        	worker.detach();
		}
    }

};

int main(){
	std::cout << "--- Booting up program ---\n";
	HttpServer myServer(8080);
	myServer.start();
	return 0;
}