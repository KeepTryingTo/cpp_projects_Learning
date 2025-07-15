#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <event.h>
#include <sys/fcntl.h>
#include <unistd.h>

void fifo_read(int fd , short events, void *arg){
    char buf[128] = {0};
    int ret = read(fd, buf, sizeof(buf));
    if(ret == -1){
        perror("read is failed!");
        exit(1);
    }
    printf("read data is %s\n", buf);
}

int main(){
    // 创建有名管道
    int ret = mkfifo("fifo.tmp", 00700);
    if(ret == -1){
        perror("mkfifo is failed!");
        exit(1);
    }

    int fd = open("fifo.tmp", O_RDONLY);
    if(fd == -1){
        perror("open fifo is failed!");
        exit(1);
    }

    //初始化事件集合
    event_init();

    // 定时事件
    struct event ev;

    //初始化事件
    // 参数： 事件集合，关联的文件描述符，事件类型，回调函数，回调函数参数
    event_set(&ev, fd, EV_READ, fifo_read, NULL);

    //把事件添加到集合中
    event_add(&ev, NULL);

    //开始监听（这个监听过程是一个不断的循环，如果集合中没有事件发生就可以监听）
    event_dispatch();

    return 0;
}