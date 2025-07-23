#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9876

// 大写转换回调
void read_callback_uppercase(struct bufferevent *bev, void *ctx) {
    char tmp[128];
    size_t n;
    int i;
    while (1) {
        // 从缓冲区中读取数据并将字符转换为大写
        n = bufferevent_read(bev, tmp, sizeof(tmp));
        if (n <= 0)
            break;
        for (i = 0; i < n; ++i)
            tmp[i] = toupper(tmp[i]);
        // 将转换为大写的字符又发送回去
        bufferevent_write(bev, tmp, n);
    }
}

// 代理数据结构
struct proxy_info {
    struct bufferevent *other_bev;
};

// 代理回调
void read_callback_proxy(struct bufferevent *bev, void *ctx) {
    struct proxy_info *inf = ctx;
    // 抽空缓冲区的所有内容
    bufferevent_read_buffer(bev, bufferevent_get_output(inf->other_bev));
}

// 斐波那契数据结构
struct count {
    unsigned long last_fib[2];
};

// 斐波那契回调
void write_callback_fibonacci(struct bufferevent *bev, void *ctx) {
    struct count *c = ctx;
    // 创建新的临时缓冲区
    struct evbuffer *tmp = evbuffer_new();
    
    // 斐波那契数列
    while (evbuffer_get_length(tmp) < 1024) {
        unsigned long next = c->last_fib[0] + c->last_fib[1];
        c->last_fib[0] = c->last_fib[1];
        c->last_fib[1] = next;
        // 格式化输出到缓冲区
        evbuffer_add_printf(tmp, "%lu ", next);
    }

    // 移除 buf 的所有内 容,将其放置到输出缓冲区的末尾（发送所有的内容）
    bufferevent_write_buffer(bev, tmp);
    evbuffer_free(tmp);

    sleep(5);
}

// 通用事件回调
void event_callback(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", 
               evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
    }
    
    bufferevent_free(bev);
    if (ctx) free(ctx);
}

// 创建监听器回调
void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd,
                   struct sockaddr *address, int socklen, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int mode = *(int *)ctx;  // 获取运行模式
    
    struct bufferevent *bev = bufferevent_socket_new(base, fd, 
                                                   BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error creating bufferevent!");
        return;
    }

    switch (mode) {
        // 大写模式
        case 1: {  
            bufferevent_setcb(bev, read_callback_uppercase, NULL, 
                            event_callback, NULL);
            bufferevent_enable(bev, EV_READ | EV_WRITE);
            bufferevent_write(bev, "Uppercase mode - type something:\n", 32);
            break;
        }
         // 代理模式
        case 2: { 
            // 创建另一个bufferevent作为目标
            struct bufferevent *target = bufferevent_socket_new(base, -1, 
                                                              BEV_OPT_CLOSE_ON_FREE);
            // 这里应该连接到实际的目标服务器
            // 为简化示例，我们只是回环到自身
            struct sockaddr_in sin;
            memset(&sin, 0, sizeof(sin));
            sin.sin_family = AF_INET;
            sin.sin_port = htons(PORT+1);
            sin.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1
            
            if (bufferevent_socket_connect(target, 
                                         (struct sockaddr *)&sin, sizeof(sin)) < 0) {
                bufferevent_free(bev);
                bufferevent_free(target);
                return;
            }
            
            struct proxy_info *inf = malloc(sizeof(struct proxy_info));
            inf->other_bev = target;
            
            bufferevent_setcb(bev, read_callback_proxy, NULL, 
                            event_callback, inf);
            bufferevent_setcb(target, read_callback_proxy, NULL, 
                            event_callback, NULL);
            
            bufferevent_enable(bev, EV_READ);
            bufferevent_enable(target, EV_READ|EV_WRITE);
            break;
        }
        // 斐波那契模式
        case 3: {  
            struct count *c = malloc(sizeof(struct count));
            c->last_fib[0] = 0;
            c->last_fib[1] = 1;
            
            bufferevent_setcb(bev, NULL, write_callback_fibonacci, event_callback, c);
            bufferevent_enable(bev, EV_WRITE);
            
            // 触发第一次写入
            write_callback_fibonacci(bev, c);
            break;
        }
    }
}

void accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mode>\n"
                "Modes:\n"
                "  1 - Uppercase echo server\n"
                "  2 - Proxy server\n"
                "  3 - Fibonacci server\n", argv[0]);
        return 1;
    }
    
    int mode = atoi(argv[1]);
    if (mode < 1 || mode > 3) {
        fprintf(stderr, "Invalid mode. Choose 1, 2 or 3.\n");
        return 1;
    }
    
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    // create socket fd -> bind -> listen
    listener = evconnlistener_new_bind(base, accept_conn_cb, &mode,
                                     LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                     -1, (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return 1;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);

    const char *modes[] = {"", "Uppercase echo", "Proxy", "Fibonacci"};
    printf("Server running in %s mode on port %d\n", modes[mode], PORT);
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}