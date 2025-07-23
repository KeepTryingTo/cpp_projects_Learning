#include <event2/event.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* 原始回调函数 */
void original_callback(evutil_socket_t fd, short what, void *arg) {
    printf("Original callback called on fd %d\n", (int)fd);
}

/* 新回调函数 */
void new_callback(evutil_socket_t fd, short what, void *arg) {
    printf("New callback called on fd %d with arg: %s\n", 
           (int)fd, (char*)arg);
}

/* 替换回调的函数 */
int replace_callback(struct event *ev, event_callback_fn new_callback,
                    void *new_callback_arg) {
    struct event_base *base;
    evutil_socket_t fd;
    short events;

    int pending = event_pending(ev, EV_READ | EV_WRITE | EV_SIGNAL | EV_TIMEOUT, NULL);
    if (pending) {
        fprintf(stderr, "Error! replace_callback called on a pending event!\n");
        return -1;
    }

    event_get_assignment(ev, &base, &fd, &events, NULL, NULL);

    event_assign(ev, base, fd, events, new_callback, new_callback_arg);
    return 0;
}

/* 创建监听套接字 */
evutil_socket_t create_listening_socket(int port) {
    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
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

int main() {
    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    /* 创建监听套接字 */
    evutil_socket_t fd = create_listening_socket(9000);
    printf("Listening on port 9000\n");

    /* 创建初始事件 */
    struct event *ev = event_new(base, fd, EV_READ | EV_PERSIST, 
                               original_callback, NULL);
    if (!ev) {
        fprintf(stderr, "Could not create event!\n");
        close(fd);
        event_base_free(base);
        return 1;
    }

    /* 添加事件到事件循环 */
    event_add(ev, NULL);

    /* 模拟事件循环运行 */
    printf("Event loop running (press Enter to replace callback)...\n");
    getchar();  // 等待用户输入

    /* 替换回调函数 */
    if (replace_callback(ev, new_callback, "Custom argument") == 0) {
        printf("Callback replaced successfully!\n");
    } else {
        printf("Failed to replace callback\n");
    }

    /* 再次模拟事件循环运行 */
    printf("Event loop running with new callback (press Enter to exit)...\n");
    getchar();

    /* 触发事件来演示回调被替换 */
    printf("Triggering event manually...\n");
    event_active(ev, EV_READ, 0);

    /* 清理 */
    event_free(ev);
    close(fd);
    event_base_free(base);

    return 0;
}