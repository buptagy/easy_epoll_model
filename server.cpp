#include <sys/epoll.h>    // epoll functions and data structures
#include <unistd.h>       // close function
#include <fcntl.h>        // fcntl function
#include <cstring>        // memset function
#include <iostream>       // standard I/O stream
#include <stdexcept>      // standard exceptions
#include <sys/socket.h>
#include <netinet/in.h>   // sockaddr_in, INADDR_ANY
#include <arpa/inet.h>    // inet_ntoa

#define MAX_EVENTS 10
#define BIND_PORT 10086

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BIND_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        close(server_fd);
        throw std::runtime_error("Failed to create epoll file descriptor");
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        close(server_fd);
        close(epoll_fd);
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_fds == -1) {
            close(server_fd);
            close(epoll_fd);
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < num_fds; ++i) {
            if (events[i].data.fd == server_fd) {
                sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    std::cerr << "Failed to accept new connection" << std::endl;
                    continue;
                }

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    close(client_fd);
                    std::cerr << "Failed to add client file descriptor to epoll" << std::endl;
                    continue;
                }

                std::cout << "Accepted new connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
            } else {
                char buffer[1024];
                ssize_t count = read(events[i].data.fd, buffer, sizeof(buffer));
                if (count == -1) {
                    std::cerr << "Read error on fd " << events[i].data.fd << std::endl;
                    close(events[i].data.fd);
                    continue;
                } else if (count == 0) {
                    std::cout << "Client disconnected on fd " << events[i].data.fd << std::endl;
                    close(events[i].data.fd);
                } else {
                    std::cout << "Read " << count << " bytes: " << std::string(buffer, count) << std::endl;
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
