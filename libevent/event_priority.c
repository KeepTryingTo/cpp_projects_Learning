#include <event2/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// 回调函数声明
void read_cb(evutil_socket_t fd, short what, void *arg);
void write_cb(evutil_socket_t fd, short what, void *arg);

// 创建并返回一个连接套接字
evutil_socket_t create_connected_socket(int port)
{
    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        exit(1);
    }

    return fd;
}

// 读回调函数
void read_cb(evutil_socket_t fd, short what, void *arg)
{
    printf("Read callback (lower priority) triggered\n");
    
    // 模拟读取数据
    char buf[1024];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        if (n == 0) {
            printf("Connection closed\n");
        } else {
            perror("recv");
        }
        return;
    }
    
    printf("Received %zd bytes\n", n);
}

// 写回调函数
void write_cb(evutil_socket_t fd, short what, void *arg)
{
    printf("Write callback (higher priority) triggered\n");
    
    // 模拟写入数据
    const char *msg = "Hello from libevent!\n";
    ssize_t n = send(fd, msg, strlen(msg), 0);
    if (n < 0) {
        perror("send");
        return;
    }
    
    printf("Sent %zd bytes\n", n);
    
    // 为了演示，我们只写一次就删除写事件
    struct event *ev = (struct event *)arg;
    if (ev) {
        event_del(ev);
        event_free(ev);
    }
}

void main_loop(evutil_socket_t fd)
{
    struct event *important, *unimportant;
    struct event_base *base;

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }

    // 初始化优先级系统，设置2个优先级级别(0和1)
    if (event_base_priority_init(base, 2) != 0) {
        fprintf(stderr, "Could not set priority levels!\n");
        event_base_free(base);
        return;
    }

    // 设置套接字为非阻塞模式
    evutil_make_socket_nonblocking(fd);

    // 创建事件：
    // - 重要事件(写): 优先级0(更高)
    // - 不重要事件(读): 优先级1(更低)
    important = event_new(base, fd, EV_WRITE, write_cb, NULL);
    unimportant = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

    if (!important || !unimportant) {
        fprintf(stderr, "Could not create events!\n");
        if (important) event_free(important);
        if (unimportant) event_free(unimportant);
        event_base_free(base);
        return;
    }

    // 设置事件优先级
    event_priority_set(important, 0);    // 更高优先级
    event_priority_set(unimportant, 1);  // 更低优先级

    // 添加事件到事件循环
    event_add(important, NULL);
    event_add(unimportant, NULL);

    printf("Starting event loop...\n");
    printf("Write events (priority 0) will be processed before read events (priority 1)\n");
    
    event_base_dispatch(base);

    // 清理
    event_free(important);
    event_free(unimportant);
    event_base_free(base);
}

int main()
{
    // 创建一个连接到本地回环地址的套接字
    evutil_socket_t fd = create_connected_socket(9000);
    printf("Connected to port 9000\n");

    // 进入主事件循环
    main_loop(fd);

    // 关闭套接字
    close(fd);

    return 0;
}