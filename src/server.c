#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "hamming.h"

#define PORT 2222
#define BUFFER_SIZE 1024

#define CALC_DIV_BY_ZERO -1
#define CALC_INVALID_OPERATOR -2

int calculate(double op1, double op2, char operation, float* res) {
	switch(operation) {
		case '+':
			*res = op1 + op2;
			break;
		case '-':
			*res = op1 - op2;
			break;
		case '*':
			*res = op1 * op2;
			break;
		case '/':
			if(op2 == 0) return CALC_DIV_BY_ZERO;
			*res = op1 / op2;
			break;
		default: return CALC_INVALID_OPERATOR;
	}
	return 0;
}

int main (void) {
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Failed to create socket");
		return -1;
	}

	// Setup server address and port
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Failed to bind socket");
		return -1;
	}
	
	printf("Calculator server running on port %d...\n", PORT);

	// Listen for exactly one connection
	if (listen(sock, 1) < 0) {
		perror("Failed to listen for connections");
		return -1;
	}

	// Accept connection
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	if ((client_fd = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
		perror("Failed to accept connection");
		return -1;
	}

	// Receive operation and Hamming decode it
	char msg[BUFFER_SIZE] = {0}, hamming[BUFFER_SIZE] = {0};
	size_t msg_len = BUFFER_SIZE, hamming_len;
	if ((hamming_len = recv(client_fd, hamming, BUFFER_SIZE, 0)) < 0) {
		perror("Failed to receive operation");
		return -1;
	}
	hamming_decode((uint8_t*)hamming, hamming_len, (uint8_t*)msg, &msg_len);

	printf("Received: %s\n", msg);

	float op1, op2;
	char op = '+', response[BUFFER_SIZE] = {0};
	if (sscanf(msg, "[%f,%f,%c]", &op1, &op2, &op) != 3) {
        snprintf(response, BUFFER_SIZE, "Error: Unrecognized syntax. Expected: [operand1,operand2,operator]");
    }

	// Evaluate operation
	float result;
	switch (calculate(op1, op2, op, &result)) {
		case 0:
			snprintf(response, BUFFER_SIZE, "%f", result);
			break;
		case CALC_DIV_BY_ZERO:
			snprintf(response, BUFFER_SIZE, "Error: Division by zero.");
			break;
		case CALC_INVALID_OPERATOR:
			snprintf(response, BUFFER_SIZE, "Error: Unrecognized operator `%c`. Expected +, -, * or /.", op);
			break;
		default: break;
	};
	printf("Result: %s\n", response);

	// Encode 
	memset(hamming, 0, BUFFER_SIZE);
	hamming_len = BUFFER_SIZE;
	size_t response_len = strlen(response)+1;
	hamming_encode((uint8_t*) response, response_len, (uint8_t*)hamming, &hamming_len);

	send(client_fd, hamming, hamming_len, 0);

	close(client_fd);
	close(sock);
	return 0;
}
