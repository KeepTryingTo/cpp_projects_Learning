#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

const int PORT = 8080;
const int MAX_EVENTS = 1024;
const int BUFFER_SIZE = 1024;

// 设置非阻塞 Socket
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    // 监听的socket fd，epoll fd和建立连接的socket fd
    int server_socket, epoll_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 1. 创建 Socket，并且设置为非阻塞
    server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 绑定 IP 和端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 3. 监听连接
    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // 4. 创建 epoll 实例,这里的0没有实际的意义
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Epoll creation failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 5. 将监听 Socket 加入 epoll结构中
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // 边缘触发模式
    ev.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
        perror("Epoll_ctl failed");
        close(server_socket);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    // 6. 事件循环
    struct epoll_event events[MAX_EVENTS];
    while (true) {
        // 阻塞等待事件（监听来自客户端的连接和发送的数据），并返回发生了事件的连接
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); 
        if (nfds == -1) {
            perror("Epoll_wait failed");
            break;
        }

        // 遍历发生事件的所有连接
        for (int i = 0; i < nfds; ++i) {
            // 如果是客户端发送请求建立新连接
            if (events[i].data.fd == server_socket) {
                while (true) {
                    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (client_socket == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // 没有更多连接
                        } else {
                            perror("Accept failed");
                            break;
                        }
                    }
                    
                    // 同时设置当前建立连接的socket fd为非阻塞
                    std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;
                    set_nonblocking(client_socket);

                    // 将客户端 Socket 加入 epoll
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_socket;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
                        perror("Epoll_ctl client failed");
                        close(client_socket);
                    }
                }
            }
            // 客户端数据可读
            else if (events[i].events & EPOLLIN) {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read;

                // 循环从缓冲区中读取接收的数据
                while ((bytes_read = recv(events[i].data.fd, buffer, BUFFER_SIZE, 0)) > 0) {
                    std::cout << "Received from client: " << buffer << std::endl;
                    const char* response = "Message received by server!";
                    //同时发送消息给客户端
                    send(events[i].data.fd, response, strlen(response), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                }

                // 客户端断开连接
                if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN)) {
                    std::cout << "Client disconnected." << std::endl;
                    close(events[i].data.fd);
                }
            }
        }
    }

    close(server_socket);
    close(epoll_fd);
    return 0;
}