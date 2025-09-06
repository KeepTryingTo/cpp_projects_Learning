#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <thread>
#include <atomic>

class EpollClient
{
private:
    int sockfd_;
    int epoll_fd_;
    struct sockaddr_in server_addr_;
    std::atomic<bool> running_;
    const int BUFFER_SIZE = 1024;

public:
    EpollClient(const std::string &ip, int port) : running_(true)
    {
        // 创建socket
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ == -1)
        {
            throw std::runtime_error("Failed to create socket");
        }

        // 设置服务器地址
        server_addr_.sin_family = AF_INET;
        server_addr_.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &server_addr_.sin_addr) <= 0)
        {
            throw std::runtime_error("Invalid address");
        }

        // 创建epoll实例
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1)
        {
            throw std::runtime_error("Failed to create epoll");
        }
    }

    ~EpollClient()
    {
        running_ = false;
        close(sockfd_);
        close(epoll_fd_);
    }

    bool connect_to_server()
    {
        if (connect(sockfd_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) == -1)
        {
            perror("connect");
            return false;
        }

        // 设置非阻塞
        set_nonblocking(sockfd_);

        // 添加socket到epoll
        add_to_epoll(sockfd_, EPOLLIN);

        std::cout << "Connected to server " << inet_ntoa(server_addr_.sin_addr)
                  << ":" << ntohs(server_addr_.sin_port) << std::endl;
        return true;
    }

    void run()
    {
        // 启动网络接收线程
        std::thread network_thread(&EpollClient::network_loop, this);

        // 主线程处理用户输入
        input_loop();

        // 等待网络线程结束
        running_ = false;
        network_thread.join();
    }

private:
    void set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

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

    void network_loop()
    {
        struct epoll_event events[10];
        char buffer[BUFFER_SIZE];

        while (running_)
        {
            int num_events = epoll_wait(epoll_fd_, events, 10, 100); // 100ms超时
            if (num_events == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < num_events; ++i)
            {
                if (events[i].data.fd == sockfd_)
                {
                    if (events[i].events & EPOLLIN)
                    {
                        handle_server_data();
                    }
                    if (events[i].events & (EPOLLERR | EPOLLHUP))
                    {
                        std::cout << "Server disconnected or error occurred" << std::endl;
                        running_ = false;
                        return;
                    }
                }
            }
        }
    }

    void handle_server_data()
    {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while (true)
        {
            bytes_read = recv(sockfd_, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break; // 没有更多数据
                }
                perror("recv");
                running_ = false;
                break;
            }
            else if (bytes_read == 0)
            {
                std::cout << "Server disconnected" << std::endl;
                running_ = false;
                break;
            }

            buffer[bytes_read] = '\0';
            std::cout << "Server: " << buffer << std::endl;
        }
    }

    void input_loop()
    {
        std::cout << "Type 'quit' to exit" << std::endl;

        while (running_)
        {
            std::string input;
            std::getline(std::cin, input);

            if (!running_)
                break;

            if (input == "quit")
            {
                std::cout << "Disconnecting..." << std::endl;
                running_ = false;
                break;
            }

            if (send(sockfd_, input.c_str(), input.size(), 0) == -1)
            {
                perror("send");
                running_ = false;
                break;
            }
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
        return 1;
    }

    try
    {
        EpollClient client(argv[1], std::stoi(argv[2]));

        if (!client.connect_to_server())
        {
            return 1;
        }

        client.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}