#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1"; // 本地回环地址
const int BUFFER_SIZE = 1024;

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // 1. 创建 Socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 设置服务端地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // 3. 连接服务端
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << PORT << std::endl;

    while (true) {
        std::string message;
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        // 4. 发送数据
        send(client_socket, message.c_str(), message.size(), 0);

        // 5. 接收回复
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Server disconnected." << std::endl;
            break;
        }
        std::cout << "Server response: " << buffer << std::endl;
    }

    close(client_socket);
    return 0;
}