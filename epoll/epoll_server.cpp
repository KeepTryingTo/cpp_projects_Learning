#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <functional>

class EpollServer
{
private:
    int server_fd_;
    int epoll_fd_;
    struct sockaddr_in server_addr_;
    std::map<int, std::string> client_buffers_;
    const int MAX_EVENTS = 64;
    const int BUFFER_SIZE = 1024;

public:
    EpollServer(int port)
    {
        // 创建服务器socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == -1)
        {
            throw std::runtime_error("Failed to create socket");
        }

        // 设置socket选项，避免地址占用
        int opt = 1;
        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options");
        }

        // 绑定地址
        server_addr_.sin_family = AF_INET;
        server_addr_.sin_addr.s_addr = INADDR_ANY;
        server_addr_.sin_port = htons(port);

        if (bind(server_fd_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) == -1)
        {
            throw std::runtime_error("Failed to bind");
        }

        // 监听
        if (listen(server_fd_, SOMAXCONN) == -1)
        {
            throw std::runtime_error("Failed to listen");
        }

        // 创建epoll实例
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1)
        {
            throw std::runtime_error("Failed to create epoll");
        }

        // 添加服务器socket到epoll
        add_to_epoll(server_fd_, EPOLLIN);
    }

    ~EpollServer()
    {
        close(server_fd_);
        close(epoll_fd_);
    }

    void run()
    {
        std::cout << "Server started on port " << ntohs(server_addr_.sin_port) << std::endl;

        struct epoll_event events[MAX_EVENTS];

        while (true)
        {
            int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
            if (num_events == -1)
            {
                perror("epoll_wait");
                continue;
            }

            for (int i = 0; i < num_events; ++i)
            {
                int fd = events[i].data.fd;

                if (fd == server_fd_)
                {
                    handle_new_connection();
                }
                else
                {
                    if (events[i].events & EPOLLIN)
                    {
                        handle_client_data(fd);
                    }
                    if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
                    {
                        handle_client_disconnect(fd);
                    }
                }
            }
        }
    }

private:
    void add_to_epoll(int fd, uint32_t events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("epoll_ctl add");
        }
    }

    void modify_epoll(int fd, uint32_t events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            perror("epoll_ctl mod");
        }
    }

    void remove_from_epoll(int fd)
    {
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            perror("epoll_ctl del");
        }
    }

    void set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void handle_new_connection()
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1)
        {
            perror("accept");
            return;
        }

        set_nonblocking(client_fd);
        add_to_epoll(client_fd, EPOLLIN | EPOLLET); // 边缘触发模式

        std::cout << "New connection from: " << inet_ntoa(client_addr.sin_addr)
                  << ":" << ntohs(client_addr.sin_port) << std::endl;

        // 发送欢迎消息
        std::string welcome = "Welcome to epoll server!\n";
        send(client_fd, welcome.c_str(), welcome.size(), 0);
    }

    void handle_client_data(int client_fd)
    {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while (true)
        {
            bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 没有更多数据可读
                    break;
                }
                perror("recv");
                handle_client_disconnect(client_fd);
                return;
            }
            else if (bytes_read == 0)
            {
                // 客户端断开连接
                handle_client_disconnect(client_fd);
                return;
            }

            buffer[bytes_read] = '\0';

            // 处理接收到的数据
            std::string message(buffer);
            std::cout << "Received from client " << client_fd << ": " << message;

            // 回显给客户端
            std::string response = "Echo: " + message;
            send(client_fd, response.c_str(), response.size(), 0);

            // 如果是退出命令
            if (message.find("quit") != std::string::npos)
            {
                handle_client_disconnect(client_fd);
                return;
            }
        }
    }

    void handle_client_disconnect(int client_fd)
    {
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        remove_from_epoll(client_fd);
        close(client_fd);
        client_buffers_.erase(client_fd);
    }
};

int main()
{
    try
    {
        EpollServer server(8080);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}