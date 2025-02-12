#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <ip> <port> <deposit/withdraw> <amount> <times>\n", argv[0]);
        exit(1);
    }
    
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    const char *operation = argv[3];
    int amount = atoi(argv[4]);
    int times = atoi(argv[5]);
    
    // check operation validity
    if (strcmp(operation, "deposit") != 0 && strcmp(operation, "withdraw") != 0) {
        fprintf(stderr, "Invalid operation. Use 'deposit' or 'withdraw'\n");
        exit(1);
    }
    
    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }
    
    // configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }
    
    // connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }
    
    // perform operations
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "%s %d\n", operation, amount);
    
    for (int i = 0; i < times; i++) {
        // send request
        if (send(sockfd, buffer, BUFFER_SIZE , 0) == -1) {
            perror("send");
            exit(1);
        }
        
        // small delay between operations
        usleep(1000);
    }
    
    close(sockfd);
    return 0;
}