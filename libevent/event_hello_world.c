#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

static const char MESSAGE[] = "Hello, World!\n";
static const int PORT = 9995;

// 函数声明
static void listener_cb(struct evconnlistener *, evutil_socket_t,
                      struct sockaddr *, int socklen, void *);

static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

// 全局变量用于资源清理
static struct event_base *base = NULL;
static struct evconnlistener *listener = NULL;
static struct event *signal_event = NULL;

// 初始化网络库（Windows需要）
static void init_network() {
#ifdef WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        exit(1);
    }
#endif
}

// 清理网络库
static void cleanup_network() {
#ifdef WIN32
    WSACleanup();
#endif
}

// 统一错误处理
static void die(const char *msg) {
    perror(msg);
    if (listener) evconnlistener_free(listener);
    if (signal_event) event_free(signal_event);
    if (base) event_base_free(base);
    cleanup_network();
    exit(1);
}

int main(int argc, char **argv) {
    struct sockaddr_in sin;

    init_network();

    // 1. 创建event_base
    base = event_base_new();
    if (!base) {
        die("Could not initialize libevent!");
    }

    // 2. 设置监听地址
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);  // 监听所有接口

    // 3. 创建监听器
    listener = evconnlistener_new_bind(
        base, listener_cb, (void *)base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr *)&sin, sizeof(sin));

    if (!listener) {
        die("Could not create listener");
    }

    // 4. 设置信号处理 (Ctrl+C)
    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    // 同时添加信号事件
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        die("Could not create/add signal event");
    }

    printf("Server running on port %d\n", PORT);

    // 5. 进入主事件循环
    event_base_dispatch(base);

    // 6. 清理资源
    printf("Server shutting down...\n");
    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);
    cleanup_network();

    return 0;
}

// 新连接回调
static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                       struct sockaddr *sa, int socklen, void *user_data) {
    
    struct event_base *base = (struct event_base *)user_data;
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in *sin = (struct sockaddr_in *)sa;

    // 获取客户端IP
    inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
    printf("New connection from %s:%d\n", ip, ntohs(sin->sin_port));

    // 创建bufferevent
    struct bufferevent *bev = bufferevent_socket_new(
        base, fd, BEV_OPT_CLOSE_ON_FREE
    );
    if (!bev) {
        fprintf(stderr, "Error creating bufferevent");
        event_base_loopbreak(base);
        return;
    }

    // 设置发送数据和监控事件回调
    bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_WRITE);  // 只启用写事件（启用发送数据事件）
    bufferevent_disable(bev, EV_READ);  // 禁用读事件 （关闭接收数据事件）

    // 发送欢迎消息
    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}

// 写回调
static void conn_writecb(struct bufferevent *bev, void *user_data) {
    struct evbuffer *output = bufferevent_get_output(bev);

    // 关闭当前连接
    if (evbuffer_get_length(output) == 0) {
        printf("Message sent, closing connection\n");
        bufferevent_free(bev);  // 自动关闭socket
    }
}

// 事件回调
static void conn_eventcb(struct bufferevent *bev, short events, void *user_data) {
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed by client\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Connection error: %s\n", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
    }
    bufferevent_free(bev);  // 释放资源
}

// 信号处理回调
static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base *)user_data;
    struct timeval delay = {2, 0};  // 2秒后退出

    printf("\nCaught interrupt signal, shutting down gracefully...\n");
    event_base_loopexit(base, &delay);
}