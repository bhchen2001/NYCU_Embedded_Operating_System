#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

int sockfd;

void zombie_handler(int sig) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void shutdown_handler(int sig) {
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /*
     *  Handle SIGCHLD signal to prevent zombie process
     *  Handle SIGINT signal to shutdown the server (CTRL-C)
     */
    signal(SIGCHLD, zombie_handler);
    signal(SIGINT, shutdown_handler);

    /*
     *  Create a socket
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    /*
     *  Allow reuse of address
     */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    /*
     *  Setup server address
     */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    /*
     *  Bind the socket to the address
     */
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    /*
     *  Listen for incoming connections
     */
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server is running on port %s\n", argv[1]);

    while (1) {
        /*
         *  Accept incoming connection
         */
        int client_fd = accept(sockfd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            exit(1);
        }

        /*
         *  Fork a child process to handle the connection
         */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) { // child process
            close(sockfd);

            printf("Train ID: %d\n", getpid());

            /*
             *  Redirect stdin to client socket
             *      - STDOUT_FILENO is now points to the client socket
             *      - client_fd becomes the duplicate (redundant) pointer to client socket
             */
            dup2(client_fd, STDOUT_FILENO);
            close(client_fd);
            
            /*
             *  sl command and error handling
             */
            execlp("/usr/games/sl", "sl", "-l", NULL);
            perror("execlp");
            exit(1);
        }
        close(client_fd);
    }

    return 0;
}