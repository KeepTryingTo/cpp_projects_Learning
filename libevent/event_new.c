#include <event2/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// 回调函数，当事件触发时被调用
void cb_func(evutil_socket_t fd, short what, void *arg)
{
    const char *data = (const char *)arg;
    printf("Got an event on socket %d:%s%s%s%s [%s]\n",
        (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "",
        data
    );
}

// 创建并返回一个监听套接字
evutil_socket_t create_listening_socket(int port)
{
    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置SO_REUSEADDR选项
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(fd);
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        exit(1);
    }

    if (listen(fd, 10) < 0) {
        perror("listen");
        close(fd);
        exit(1);
    }

    return fd;
}

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

void main_loop(evutil_socket_t fd1, evutil_socket_t fd2)
{
    struct event *ev1, *ev2;
    struct timeval five_seconds = {5,0};
    struct event_base *base = event_base_new();

    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }

    // 设置套接字为非阻塞模式
    evutil_make_socket_nonblocking(fd1);
    evutil_make_socket_nonblocking(fd2);

    // 创建事件：fd1可读，带5秒超时，持久事件
    ev1 = event_new(base, fd1, EV_TIMEOUT | EV_READ | EV_PERSIST, cb_func,
       (char*)"Reading event");

    // 创建事件：fd2可写，持久事件
    ev2 = event_new(base, fd2, EV_WRITE | EV_PERSIST, cb_func,
       (char*)"Writing event");

    if (!ev1 || !ev2) {
        fprintf(stderr, "Could not create event!\n");
        return;
    }

    // 添加事件到事件循环
    event_add(ev1, &five_seconds);
    event_add(ev2, NULL);

    printf("Starting event loop...\n");
    event_base_dispatch(base);

    // 清理
    event_free(ev1);
    event_free(ev2);
    event_base_free(base);
}

int main()
{
    // 创建两个套接字
    evutil_socket_t fd1 = create_listening_socket(9000);  // 读套接字
    evutil_socket_t fd2 = create_connected_socket(9001);  // 写套接字

    printf("Listening on port 9000, connected to port 9001\n");

    // 进入主事件循环
    main_loop(fd1, fd2);

    // 关闭套接字
    close(fd1);
    close(fd2);

    return 0;
}