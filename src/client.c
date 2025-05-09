#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "hamming.h"

#define BUFFER_SIZE 1024

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
	char operator, answer, msg[BUFFER_SIZE], hamming[BUFFER_SIZE];
	size_t hamming_len, msg_len;
	int repeat = 1;
	while (repeat) {
		memset(msg, 0, BUFFER_SIZE);
		memset(hamming, 0, BUFFER_SIZE);
		hamming_len = BUFFER_SIZE;

		printf("Operand 1? ");
		scanf("%f", &op1);
		printf("Operand 2? ");
		scanf("%f", &op2);
		printf("Operator [+, -, *, /]? ");
		scanf(" %c", &operator);
	
		// Send operation after Hamming encoding
		msg_len = snprintf(msg, sizeof(msg), "[%f,%f,%c]", op1, op2, operator);
		hamming_encode((uint8_t*)msg, msg_len, (uint8_t*)hamming, &hamming_len);
	
		if (send(sock, hamming, hamming_len, 0) < 0) {
			perror("Failed to send operation");
			return -1;
		}
	
		// Receive result and Hamming decode it
		memset(hamming, 0, hamming_len);
		if ((hamming_len = recv(sock, hamming, BUFFER_SIZE, 0)) < 0) {
			perror("Failed to receive response");
			return -1;
		}
	
		msg_len = BUFFER_SIZE;
		hamming_decode((uint8_t*)hamming, hamming_len, (uint8_t*)msg, &msg_len);
		
		printf("Result from server: %s\n", msg);

		printf("Another operation? [y/n] ");
		scanf(" %c", &answer);
		repeat = (answer == 'y');

		if (!repeat) {
			snprintf(msg, sizeof("exit"), "exit");
			hamming_encode((uint8_t*)msg, msg_len, (uint8_t*)hamming, &hamming_len);
			if (send(sock, hamming, hamming_len, 0) < 0) {
				perror("Failed to send exit message");
				return -1;
			}
		}

	}

	close(sock);
	return 0;
}
