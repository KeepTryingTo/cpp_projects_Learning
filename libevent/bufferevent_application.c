#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/listener.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct info {
    const char *name;
    size_t total_drained;
};

void read_callback(struct bufferevent *bev, void *ctx)
{
    struct info *inf = ctx;
    // 获得当前的输入信息以及数据长度
    struct evbuffer *input = bufferevent_get_input(bev);
    size_t len = evbuffer_get_length(input);
    if (len) {
        inf->total_drained += len;
        // 从缓冲区头部移除指定长度的数据，通常用于表示这些数据已经被应用程序处理完毕
        evbuffer_drain(input, len);
        printf("Drained %lu bytes from %s\n",
             (unsigned long) len, inf->name);
        
        // 回显数据到输出缓冲区
        char response[256];
        snprintf(response, sizeof(response), "Echo: Received %lu bytes\n", (unsigned long)len);
        bufferevent_write(bev, response, strlen(response));
    }
}

void event_callback(struct bufferevent *bev, short events, void *ctx)
{
    struct info *inf = ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    int finished = 0;

    // 判断当前事件情况
    if (events & BEV_EVENT_EOF) {
        size_t len = evbuffer_get_length(input);
        printf("Got a close from %s. We drained %lu bytes from it, "
            "and have %lu left.\n", inf->name,
            (unsigned long)inf->total_drained, (unsigned long)len);
        finished = 1;
    }
    if (events & BEV_EVENT_ERROR) {
        printf("Got an error from %s: %s\n",
            inf->name, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = 1;
    }
    if (finished) {
        free(ctx);
        bufferevent_free(bev);
    }
}

void accept_conn_cb(struct evconnlistener *listener, 
                   evutil_socket_t fd,
                   struct sockaddr *address, 
                   int socklen,
                   void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    
    // 为新的连接创建bufferevent
    struct bufferevent *bev = bufferevent_socket_new(base, fd, 
                                                   BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }

    // 设置回调函数和水位标记
    struct info *inf = malloc(sizeof(struct info));
    inf->name = "new connection";
    inf->total_drained = 0;

    bufferevent_setwatermark(bev, EV_READ, 128, 0);
    bufferevent_setcb(bev, read_callback, NULL, event_callback, inf);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    // 发送欢迎消息
    bufferevent_write(bev, "Welcome to the echo server!\n", 27);
}

void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    // 如果发生错误，重新监听连接
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
   
    // 事件回调函数都处理完成之后退出程序
    event_base_loopexit(base, NULL);
}

struct bufferevent *setup_bufferevent(struct event_base *base, evutil_socket_t fd)
{
    struct bufferevent *b1 = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!b1) {
        fprintf(stderr, "Error creating bufferevent!");
        return NULL;
    }

    struct info *info1 = malloc(sizeof(struct info));
    info1->name = "buffer 1";
    info1->total_drained = 0;

    // 设置低水位掩码和高水位掩码
    bufferevent_setwatermark(b1, EV_READ, 128, 0);
    // 设置读事件回调以及处理事件的回调
    bufferevent_setcb(b1, read_callback, NULL, event_callback, info1);
    // 启用读或者写事件
    bufferevent_enable(b1, EV_READ | EV_WRITE);

    return b1;
}

int main(int argc, char **argv)
{
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    int port = 9000;

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                     LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                     -1, (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return 1;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);

    printf("Server running on port %d\n", port);
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}