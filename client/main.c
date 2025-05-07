#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [server ip] [server port]", argv[0]);
        return -1;
    }

    uint16_t server_port = atoi(argv[2]);
    server_port = htons(server_port);

    int sock = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sock < 0) {
        perror("Could not create socket");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = server_port;
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) == 0) {
        fprintf(stderr, "Please provide a valid IP address.");
        return -1;
    }

    if (connect(sock, (const struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        perror("Could not connect to server");
        return -1;
    }

    float op1, op2;
    char operator;
    if (scanf("[ %f , %f , %c ]", &op1, &op2, &operator) != 3) {
        fprintf(stderr, "Unrecognized pattern. Expected input: [operand1, operand2, operator].\n");
        return -1;
    }

    printf("%f %c %f\n", op1, operator, op2);

    // === New code: format and send the message to the server ===
    char message[100];
    snprintf(message, sizeof(message), "%f,%f,%c", op1, op2, operator);
    send(sock, message, strlen(message), 0);

    // === New code: receive the response from the server ===
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Result from server: %s\n", buffer);

    // Close the socket
    close(sock);

    return 0;
}
