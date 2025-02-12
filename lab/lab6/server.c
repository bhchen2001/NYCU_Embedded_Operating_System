#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include "sem.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

static int balance = 0;
int semid = -1;
int server_sockfd = -1;

void cleanup(int sig);
void *handle_client(void *arg);

typedef struct {
    int semid;
    int client_sockfd;
} client_args_t;

void cleanup(int sig) {
    if (sem_destroy(semid) < 0) {
        perror("semctl");
        exit(1);
    }
    if (server_sockfd != -1) {
        close(server_sockfd);
    }
    exit(0);
}

void *handle_client(void *arg) {
    client_args_t *args = (client_args_t *)arg;
    int semid = args->semid;
    int client_sockfd = args->client_sockfd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    memset(buffer, 0, BUFFER_SIZE);

    while ((bytes_read = recv(client_sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        // remove trailing newline
        buffer[bytes_read - 1] = '\0';
        // printf("Received: %s\n", buffer);

        char operation[10];
        int price;
        sscanf(buffer, "%s %d", operation, &price);

        if (price < 0) {
            printf("Invalid price\n");
            continue;
        }

        // enter critical section
        if (sem_wait(semid) == -1) {
            perror("sem_wait");
            return NULL;
        }

        if (strcmp(operation, "deposit") == 0) {
            balance += price;
            printf("After deposit: %d\n", balance);
        }
        else if (strcmp(operation, "withdraw") == 0) {
            // allow the balance to go negative
            balance -= price;
            printf("After withdraw: %d\n", balance);
        }
        else {
            printf("Invalid operation\n");
        }

        // exit critical section
        if (sem_signal(semid) == -1) {
            perror("sem_signal");
            return NULL;
        }

        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // set signal handler
    signal(SIGINT, cleanup);

    semid = sem_init();
    if (semid == -1) {
        perror("sem_init");
        exit(1);
    }

    int client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // allow reuse of address
    int optval = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sockfd);
        exit(1);
    }

    if (listen(server_sockfd, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_sockfd);
        exit(1);
    }

    printf("Server listening on port %s...\n", argv[1]);

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd == -1) {
            perror("accept");
            continue;
        }

        client_args_t *args = (client_args_t *)malloc(sizeof(client_args_t));
        if (args == NULL) {
            perror("malloc");
            close(client_sockfd);
            continue;
        }

        args->semid = semid;
        args->client_sockfd = client_sockfd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void*)args) != 0) {
            perror("pthread_create");
            free(args);
            close(client_sockfd);
            continue;
        }

        pthread_detach(tid);
    }

    return 0;
}