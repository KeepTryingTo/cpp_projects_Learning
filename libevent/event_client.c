#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

int main(int argc, char *argv[])
{
    int fd = 0;
    char *str = "hello libevent!";

    fd = open("event.fifo", O_RDWR);
    if (fd < 0) {
        perror("open error");
        exit(1);
    }

    while (1) {
        write(fd, str, strlen(str));
        sleep(1);
    }

    close(fd);

        return 0;
}