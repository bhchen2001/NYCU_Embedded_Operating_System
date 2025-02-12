#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hw2.h"

void handle_client(int client_sockfd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // create socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // setup server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind socket
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("error binding socket");
        exit(1);
    }

    // listen for connections
    if (listen(server_sockfd, 5) < 0) {
        perror("error listening");
        exit(1);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        // accept client connection
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0) {
            perror("error accepting connection");
            continue;
        }

        handle_client(client_sockfd);
        close(client_sockfd);
    }

    close(server_sockfd);
    return 0;
}

void handle_client(int client_sockfd) {
    char buffer[BUFFER_SIZE];
    Order orders[RESTAURANT_MENU_NUM];
    int current_restaurant = -1;

    memset(orders, 0, sizeof(Order) * RESTAURANT_MENU_NUM);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t count = recv(client_sockfd, buffer, BUFFER_SIZE, 0);
        if (count < 0) {
            perror("error reading from socket");
            break;
        }
        else if (count == 0) {
            printf("Client disconnected\n");
            break;
        }

        // remove trailing newline
        buffer[strcspn(buffer, "\n")] = '\0';

        printf("Received command: %s\n", buffer);

        if (strcmp(buffer, "shop list") == 0) {
            send_shop_list(client_sockfd);
        }
        else if (strncmp(buffer, "order", 5) == 0) {
            if (!process_order(client_sockfd, buffer, orders, &current_restaurant)) {
                break;
            }
        }
        else if (strcmp(buffer, "confirm") == 0) {
            handle_confirm(client_sockfd, orders, current_restaurant);
            clear_orders(orders, &current_restaurant);
        }
        else if (strcmp(buffer, "cancel") == 0) {
            break;
        }
    }
}