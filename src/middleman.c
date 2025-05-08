#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define MIDDLEMAN_PORT 8000
#define BUFFER_SIZE 1024
#define CORRUPTION_PROBABILITY 10  // 10% chance

void corrupt_message(char *msg, int length) {
    if (length <= 1) return;
    int pos = rand() % (length - 1);
    msg[pos] ^= 0x01;
    printf("Middleman corrupted byte at position %d\n", pos);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [server ip] [server port]\n", argv[0]);
        exit(1);
    }

    // Initialize random seed
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));

    // Create listening socket for clients
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Configure middleman address
    struct sockaddr_in middle_addr;
    memset(&middle_addr, 0, sizeof(middle_addr));
    middle_addr.sin_family = AF_INET;
    middle_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    middle_addr.sin_port = htons(MIDDLEMAN_PORT);

    // Bind and listen
    if (bind(listen_sock, (struct sockaddr *)&middle_addr, sizeof(middle_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    if (listen(listen_sock, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Middleman running on port %d...\n", MIDDLEMAN_PORT);

    while (1) {
        // Accept new client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Create NEW server connection for each client
        int server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            perror("Server socket creation failed");
            close(client_sock);
            continue;
        }

        // Configure server address
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(argv[2]));
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);

        // Connect to server
        if (connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection to server failed");
            close(client_sock);
            close(server_sock);
            continue;
        }

        // Forward client message to server
        char buffer[BUFFER_SIZE];
        int n = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (n > 0) {
            if (rand() % 100 < CORRUPTION_PROBABILITY) {
                corrupt_message(buffer, n);
            }
            send(server_sock, buffer, n, 0);
        }

        // Forward server response back to client
        n = recv(server_sock, buffer, BUFFER_SIZE, 0);
        if (n > 0) {
            send(client_sock, buffer, n, 0);
        }

        // Close connections for this session
        close(client_sock);
        close(server_sock);
    }

    close(listen_sock);
    return 0;
}
