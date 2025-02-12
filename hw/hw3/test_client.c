#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 256

void print_menu() {
    printf("\nAvailable commands:\n");
    printf("1. shop list\n");
    printf("2. order <item> <quantity>\n");
    printf("3. confirm\n");
    printf("4. cancel\n");
    printf("5. quit\n");
    printf("Enter command: ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server at %s:%s\n", argv[1], argv[2]);

    while (1) {
        print_menu();

        // Get command from user
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = 0;  // Remove trailing newline

        // Handle quit command locally
        if (strcmp(command, "quit") == 0 || strcmp(command, "5") == 0) {
            printf("Disconnecting from server...\n");
            break;
        }

        // Convert numeric menu choices to commands
        if (strcmp(command, "1") == 0) {
            strcpy(command, "shop list\n");
        }
        else if (strcmp(command, "3") == 0) {
            strcpy(command, "confirm\n");
        }
        else if (strcmp(command, "4") == 0) {
            strcpy(command, "cancel\n");
        }

        // Send command to server
        // strcat(command, "\n");
        write(sock, command, BUFFER_SIZE);

        // If cancel command, break after sending
        if (strncmp(command, "cancel", 6) == 0) {
            printf("Order cancelled\n");
            break;
        }

        // Read response from server
        memset(buffer, 0, BUFFER_SIZE);
        
        // For shop list, need to read multiple lines
        if (strncmp(command, "shop list", 9) == 0) {
            printf("\nAvailable Restaurants and Menu Items:\n");
            int n = read(sock, buffer, BUFFER_SIZE);
            if (n > 0) {
                printf("Server Response:\n%s", buffer);
            }
        }
        // For other commands, read single response
        else {
            int n = read(sock, buffer, BUFFER_SIZE);
            if (n > 0) {
                printf("Server response: %s", buffer);
            }
            
            // If confirm command and order was successful, wait for delivery
            if (strncmp(command, "confirm", 7) == 0 && 
                strncmp(buffer, "Please wait", 11) == 0) {
                memset(buffer, 0, BUFFER_SIZE);
                n = read(sock, buffer, BUFFER_SIZE);
                if (n > 0) {
                    printf("Server response: %s", buffer);
                }
            }
        }
    }

    close(sock);
    return 0;
}