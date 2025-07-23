#include <event.h>  // 使用 libevent2 的头文件
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* 回调函数：当 watchdog_fd 可读时，调用 loopbreak 退出事件循环 */
void cb(evutil_socket_t sock, short what, void *arg) {
    struct event_base *base = arg;
    printf("Watchdog event triggered! Exiting loop.\n");
    event_base_loopbreak(base);  // 退出事件循环
}

void main_loop(struct event_base *base, evutil_socket_t watchdog_fd) {
    struct event *watchdog_event;

    /* 创建一个事件，当 watchdog_fd 可读时触发回调 cb */
    watchdog_event = event_new(base, watchdog_fd, EV_READ | EV_PERSIST, cb, base);
    if (!watchdog_event) {
        fprintf(stderr, "Failed to create event!\n");
        return;
    }

    /* 添加事件到事件循环 */
    if (event_add(watchdog_event, NULL) < 0) {
        fprintf(stderr, "Failed to add event!\n");
        return;
    }

    printf("Starting event loop... (send data to %d to exit)\n", watchdog_fd);
    event_base_dispatch(base);  // 进入事件循环

    /* 清理 */
    event_free(watchdog_event);
}

/* 创建一个本地 TCP 套接字并返回 fd */
evutil_socket_t create_watchdog_socket() {
    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket() failed"); 
        return -1;
    }

    /* 设置 SO_REUSEADDR 避免端口占用, 就是重用端口 */
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(fd);
        return -1;
    }

    /* 绑定到本地端口 9000 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);  // 监听 9000 端口 主机字节序转换为网络字节序
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1 主机字节序转换为网络字节序

    // 绑定端口
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        close(fd);
        return -1;
    }

    /* 开始监听 */
    if (listen(fd, 5) < 0) {  // backlog=5
        perror("listen() failed");
        close(fd);
        return -1;
    }

    printf("Watchdog socket listening on port 9000\n");
    return fd;
}

int main() {
    /* 初始化 libevent */
    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Failed to create event base!\n");
        return 1;
    }

    /* 创建 watchdog 套接字 */
    evutil_socket_t watchdog_fd = create_watchdog_socket();
    if (watchdog_fd < 0) {
        event_base_free(base);
        return 1;
    }

    /* 设置套接字为非阻塞（libevent 要求） 底层实现就是使用的fcntl来设置非阻塞模式*/
    if (evutil_make_socket_nonblocking(watchdog_fd) < 0) {
        perror("evutil_make_socket_nonblocking() failed");
        close(watchdog_fd);
        event_base_free(base);
        return 1;
    }

    /* 进入主事件循环 */
    main_loop(base, watchdog_fd);

    /* 清理 */
    close(watchdog_fd);
    event_base_free(base);

    printf("Program exited.\n");
    return 0;
}