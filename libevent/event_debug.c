#include <event2/event.h>
#include <event2/event_struct.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 回调函数
void cb(evutil_socket_t fd, short what, void *ptr) {
    struct event *ev = ptr;
    char buf[1024];
    ssize_t n;

    // 读取数据
    n = read(fd, buf, sizeof(buf)-1);
    if (n <= 0) {
        if (n == 0) {
            printf("Connection closed on fd %d\n", fd);
        } else {
            perror("read");
        }
        if (ev) {
            event_free(ev);  // 释放堆分配的事件
        }
        close(fd);
        return;
    }

    buf[n] = '\0';
    printf("Received on fd %d: %s\n", fd, buf);

    // 如果是堆分配的事件，可以在此处进行特殊处理
    if (ev) {
        printf("This event was heap-allocated\n");
    }
}

// 创建监听套接字
evutil_socket_t create_listener(int port) {
    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置SO_REUSEADDR 端口可重用
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        perror("setsockopt");
        close(fd);
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind is failed\n");
        close(fd);
        exit(1);
    }

    if (listen(fd, 10)) {
        perror("listen is failed\n");
        close(fd);
        exit(1);
    }

    printf("Listening on port %d\n", port);
    return fd;
}

// 主事件循环
void mainloop(evutil_socket_t fd1, evutil_socket_t fd2, int debug_mode) {
    struct event_base *base;
    struct event event_on_stack, *event_on_heap;

    if (debug_mode) {
        event_enable_debug_mode();
        printf("Debug mode enabled\n");
    }

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }

    // 堆分配的事件（需要手动释放）
    event_on_heap = event_new(base, fd1, EV_READ | EV_PERSIST, cb, NULL);
    if (!event_on_heap) {
        fprintf(stderr, "Could not create heap event!\n");
        event_base_free(base);
        return;
    }

    // 栈分配的事件（自动释放）
    if (event_assign(&event_on_stack, base, fd2, EV_READ | EV_PERSIST, cb, &event_on_stack)) {
        fprintf(stderr, "Could not assign stack event!\n");
        event_free(event_on_heap);
        event_base_free(base);
        return;
    }

    // 添加事件到事件循环
    if (event_add(event_on_heap, NULL) || event_add(&event_on_stack, NULL)) {
        fprintf(stderr, "Could not add events!\n");
        event_free(event_on_heap);
        event_base_free(base);
        return;
    }

    printf("Entering event loop...\n");
    event_base_dispatch(base);

    // 清理
    event_free(event_on_heap);
    event_base_free(base);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <debug_mode:0|1>\n", argv[0]);
        return 1;
    }

    int debug_mode = atoi(argv[1]);
    // 同时开启两个服务监听
    evutil_socket_t fd1 = create_listener(8080);
    evutil_socket_t fd2 = create_listener(8081);

    // 设置非阻塞模式（libevent要求，底层就是使用的fcntl来设置非阻塞）
    evutil_make_socket_nonblocking(fd1);
    evutil_make_socket_nonblocking(fd2);

    mainloop(fd1, fd2, debug_mode);

    close(fd1);
    close(fd2);
    return 0;
}