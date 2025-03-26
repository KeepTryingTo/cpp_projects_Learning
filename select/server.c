#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>

pthread_mutex_t mutex;

typedef struct info {
    int fd;
    int * maxfd;
    fd_set * rdset;
}FDInfo;

void * selectConn(void * arg){
    FDInfo * info = (FDInfo*)arg;
    // 接受连接请求, 这个调用不阻塞
    struct sockaddr_in cliaddr;
    int cliLen = sizeof(cliaddr);
    int cfd = accept(info -> fd, (struct sockaddr*)&cliaddr, &cliLen);
    // 得到了有效的文件描述符
    // 通信的文件描述符添加到读集合
    // 在下一轮select检测的时候, 就能得到缓冲区的状态
    pthread_mutex_lock(&mutex);
    FD_SET(cfd, info -> rdset);
    pthread_mutex_unlock(&mutex);
    
     // 重置最大的文件描述符
    *info -> maxfd = cfd > *info -> maxfd ? cfd : *info -> maxfd;
    free(info);
    return NULL;
}

void * selectRead(void * arg){
    FDInfo * info = (FDInfo*)arg;
    // 接收数据
    char buf[10] = {0};
    // 一次只能接收10个字节, 客户端一次发送100个字节
    // 一次是接收不完的, 文件描述符对应的读缓冲区中还有数据
    // 下一轮select检测的时候, 内核还会标记这个文件描述符缓冲区有数据 -> 再读一次
    // 	循环会一直持续, 知道缓冲区数据被读完位置
    int len = read(info -> fd, buf, sizeof(buf));
    if(len == 0)
    {
        printf("客户端关闭了连接...\n");
        // 将检测的文件描述符从读集合中删除
        pthread_mutex_lock(&mutex);
        FD_CLR(info -> fd, info -> rdset);
        pthread_mutex_unlock(&mutex);
        close(info -> fd);
    }else if(len > 0){
        // 收到了数据
        // 发送数据
        printf("%s\n", buf);
        write(info -> fd, buf, strlen(buf)+1);
    }else{
        // 异常
        perror("read");
    }
    free(info);
    return NULL;
}

int main()
{
    // 1. 创建监听的fd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(lfd, (struct sockaddr*)&addr, sizeof(addr));

    // 3. 设置监听
    listen(lfd, 128);

    // 将监听的fd的状态检测委托给内核检测
    int maxfd = lfd;
    // 初始化检测的读集合
    fd_set rdset;//读缓冲区文件描述符检测集合
    fd_set rdtemp;
    // 清零
    FD_ZERO(&rdset);
    // 将监听的lfd设置到检测的读集合中
    FD_SET(lfd, &rdset);
    // 通过select委托内核检测读集合中的文件描述符状态, 检测read缓冲区有没有数据
    // 如果有数据, select解除阻塞返回
    // 应该让内核持续检测
    while(1)
    {
        // 默认阻塞
        // rdset 中是委托内核检测的所有的文件描述符
        pthread_mutex_lock(&mutex);
        rdtemp = rdset;
        pthread_mutex_unlock(&mutex);
        int num = select(maxfd+1, &rdtemp, NULL, NULL, NULL);
        // rdset中的数据被内核改写了, 只保留了发生变化的文件描述的标志位上的1, 没变化的改为0
        // 只要rdset中的fd对应的标志位为1 -> 缓冲区有数据了
        // 判断
        // 有没有来自客户端的新连接
        if(FD_ISSET(lfd, &rdtemp))
        {
            //创建线程
            pthread_t tid;
            FDInfo * info = (FDInfo*)malloc(sizeof(FDInfo));
            info -> fd = lfd;
            info -> maxfd = &maxfd;
            info -> rdset = &rdset;
            pthread_create(&tid, NULL, selectConn, info);
            pthread_detach(tid);
        }

        // 没有新连接, 通信
        for(int i=0; i<maxfd+1; ++i)
        {
			// 判断从监听的文件描述符之后到maxfd这个范围内的文件描述符是否读缓冲区有数据
            //如果是通信文件描述符并且在集合中
            if(i != lfd && FD_ISSET(i, &rdtemp))
            {
                //创建线程
                pthread_t tid;
                FDInfo * info = (FDInfo*)malloc(sizeof(FDInfo));
                info -> fd = lfd;
                info -> maxfd = &maxfd;
                info -> rdset = &rdset;
                pthread_create(&tid, NULL, selectRead, info);
                pthread_detach(tid);
            }
        }
    }

    return 0;
}
