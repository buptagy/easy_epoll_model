#include <iostream>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 10086
#define SERVER_IP "127.0.0.1"
#define MESSAGE "Hello, World"
#define CLIENT_PORT 12345

int main() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(client_fd, (sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        close(client_fd);
        throw std::runtime_error("Failed to bind socket to source port");
    }
    
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        close(client_fd);
        throw std::runtime_error("Invalid address or address not supported");
    }

    if (connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(client_fd);
        throw std::runtime_error("Connection failed");
    }

    ssize_t sent_bytes = send(client_fd, MESSAGE, strlen(MESSAGE), 0);
    if (sent_bytes == -1) {
        close(client_fd);
        throw std::runtime_error("Failed to send message");
    }

    std::cout << "Sent message: " << MESSAGE << std::endl;

    close(client_fd);
    return 0;
}
