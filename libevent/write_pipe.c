#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>


int main(){
    int fd = open("fifo.tmp", O_WRONLY);
    if(fd == -1){
        perror("open fifo is failed!");
        exit(1);
    }

    char buf[32] = {0};
    while(1){
        scanf("%s", buf);
        int ret = write(fd, buf, strlen(buf));
        if(ret == -1){
            perror("wirte fifo is failed!");
            exit(1);
        }
        if(!strcmp(buf, "bye")){
            break;
        }

        memset(buf, 0, sizeof(buf));
    }

    close(fd);

    return 0;
}