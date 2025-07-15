#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <event.h>
#include <string.h>
#include <arpa/inet.h>
#include <event2/listener.h>

void read_cb(struct bufferevent * bev, void * ctx){
    int fd = bufferevent_getfd(bev);
    char buf[128] = {0};
    size_t ret = bufferevent_read(bev, buf, sizeof(buf));
    if(ret < 0){
        perror("bufferevent read is error!");
    }else{
        printf("read data fd is: %d  and data is: %s\n",fd, buf);
    }
}

void event_cb(struct bufferevent * bev, short what, void *ctx){
    if(what & BEV_EVENT_EOF){
        printf("client is offline %d\n", bufferevent_getfd(bev));
        bufferevent_free(bev);
    }else{
        perror("not know error!");
    }
}

void listener_cb(struct evconnlistener * listener, 
    evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg){
    
    printf("recv from %d\n", fd);
    struct event_base * base = arg;
    
    struct bufferevent * bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL){
        perror("bufferevent socket new is failed!");
        exit(1);
    }

    // bufferevnet对象，读事件回调函数，写事件回调函数，其他事件回调函数，参数
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);


    bufferevent_enable(bev, EV_READ);
}


int main(){
    //创建一个事件集合
    struct event_base * base = event_base_new();
    if(base == NULL){
        perror("base is created failed!");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 8000;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //创建socket，绑定，监听，接收链接
    //创建监听对象，在指定的地址上监听接下来的TCP连接
    struct evconnlistener * listener = evconnlistener_new_bind(
        base, listener_cb, base, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 10,
        (struct sockaddr *)&server_addr, sizeof(server_addr)
    );

    if(listener == NULL){
        perror("listen is failed !");
        exit(1);
    }

    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}