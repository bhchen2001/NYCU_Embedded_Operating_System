#ifndef WRITER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define SEG_DEV_DEVICE "/dev/display_7seg"
#define LED_DEV_DEVICE "/dev/display_led"

void seg_writer(int num) {
    int fd;
    int i = 0;
    char num_str[12];

    fd = open(SEG_DEV_DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open the device: %s\n", SEG_DEV_DEVICE);
        exit(1);
    }

    // convert the number to a string
    sprintf(num_str, "%d", num);
    num_str[strlen(num_str)] = '\0';

    // write a digit to the 7-segment display
    for (i = 0; i < strlen(num_str); i++) {
        write(fd, &num_str[i], 1);
        usleep(500000);
    }

    close(fd);
}


void led_writer(int num) {
    int fd;
    int i = 0;

    fd = open(LED_DEV_DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open the device: %s\n", LED_DEV_DEVICE);
        exit(1);
    }

    // write a digit to the 7-segment display
    for (i = num; i > 0; i--) {
        // convert the number to a string
        char num_str[2];
        sprintf(num_str, "%d", i);
        write(fd, &num_str[0], 1);
        usleep(1000000);
    }

    /*
     *  write a newline character to the 7-segment display
     *      - to clear the display
     */
    write(fd, "\n", 1);

    close(fd);
}

#endif