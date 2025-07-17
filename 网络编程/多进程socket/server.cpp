#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>


const int PORT = 8080;
const int BUFFER_SIZE = 1024;

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        // 接收客户端消息
        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            std::cerr << "Client disconnected or error occurred." << std::endl;
            break;
        }
        std::cout << "Received from client: " << buffer << std::endl;

        // 发送回复
        const char* response = "Message received by server!";
        send(client_socket, response, strlen(response), 0);
        memset(buffer, 0, BUFFER_SIZE); // 清空缓冲区
    }
    close(client_socket); // 关闭客户端连接
}

int main() {
    //定义监听连接的fd和建立连接的fd
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 1. 创建 Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 绑定 IP 和端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    server_addr.sin_port = htons(PORT);

    // 服务端绑定端口和地址
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 3. 监听连接
    if (listen(server_socket, 5) == -1) { // 最多 5 个等待连接
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // 4. 接受客户端连接并 fork 子进程
    while (true) {
        // 从全连接队列中获取已建立建立的socket fd
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        // 5. 创建子进程处理客户端
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            close(client_socket);
        } else if (pid == 0) { // 子进程
            // 子进程不需要监听 Socket
            close(server_socket); 
            handle_client(client_socket);
            // 处理完成后退出子进程，通知父进程回收资源
            exit(EXIT_SUCCESS); 
        } else { // 父进程
            // 父进程不需要客户端 Socket
            close(client_socket); 
            // 非阻塞等待子进程退出（避免僵尸进程）
            waitpid(-1, nullptr, WNOHANG); 
        }
    }

    close(server_socket);
    return 0;
}