/*
 *  ./writer <name>
 *  write the char every one second to the driver
 */

#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <signal.h>    // signal()
#include <fcntl.h>     // open()
#include <unistd.h>    // read(), write(), close()

#define DEV_NAME "/dev/mydev"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./writer <name>");
        exit(EXIT_FAILURE);
    }

    int fd;
    if ((fd = open(DEV_NAME, O_RDWR)) < 0) {
        perror(DEV_NAME);
        exit(EXIT_FAILURE);
    }

    int write_len = 0;
    while(1) {
        if (write_len >= strlen(argv[1])) {
            write_len = 0;
        }
        if (write(fd, &argv[1][write_len], 1) < 0) {
            perror("write()");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "write %c\n", argv[1][write_len]);
        write_len++;
        sleep(1);
    }

    close(fd);

    return 0;
}
