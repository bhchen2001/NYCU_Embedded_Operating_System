/*
 *  ./writer <student id>
 *  write the single int value to the device file every second
 */

#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <signal.h>    // signal()
#include <fcntl.h>     // open()
#include <unistd.h>    // read(), write(), close()

#define DEV_NAME "/dev/etx_device"

int main(int argc, char *argv[])
{
    int fd;
    char buf;
    int ret;
    int idx = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <student id>\n", argv[0]);
        exit(1);
    }

    fd = open(DEV_NAME, O_RDWR);
    if (fd < 0) {
        perror("open()");
        exit(1);
    }

    while (1) {
        buf = argv[1][idx];
        ret = write(fd, &buf, 1);
        if (ret < 0) {
            perror("write()");
            break;
        }
        idx = (idx + 1) % strlen(argv[1]);
        sleep(1);
    }

    close(fd);

    return 0;
}