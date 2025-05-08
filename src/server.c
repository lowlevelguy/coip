#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024

double calculate(double op1, double op2, char operation) {
    switch(operation) {
        case '+': return op1 + op2;
        case '-': return op1 - op2;
        case '*': return op1 * op2;
        case '/':
            if(op2 == 0) return NAN;
            return op1 / op2;
        default: return NAN;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Calculator server running on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        int valread = read(new_socket, buffer, BUFFER_SIZE);
        buffer[valread] = '\0';
        printf("Received: %s\n", buffer);  // Debug print

        float op1, op2;
        char op;
        if (sscanf(buffer, "%f %f %c", &op1, &op2, &op) != 3) {
            char* error_msg = "ERROR: Invalid input format";
            send(new_socket, error_msg, strlen(error_msg), 0);
            close(new_socket);
            continue;
        }

        double result = calculate(op1, op2, op);
        char response[100];
        if (isnan(result)) {
            snprintf(response, sizeof(response), "ERROR: Invalid operation");
        } else {
            snprintf(response, sizeof(response), "%.2f", result);
        }

        printf("Sending: %s\n", response);  // Debug print
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
    return 0;
}
