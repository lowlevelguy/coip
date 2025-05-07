#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main (int argc, char** argv) {
	// Check that the program is being called correctly
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [server ip] [server port]", argv[0]);
		return -1;
	}

	// Check that the provided port is valid. There's no need to manually
	// verify the IP address, as inet_pton does that on its own.
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

	if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0) {
		perror("Could not connect to server");
		return -1;
	}

	// Getting arithmetic operation from input
	float op1, op2;
	char operator = 0, req[256] = {0}, res[256] = {0};
	
	printf("First operand? ");
	scanf("%f", &op1);

	printf("Second operand? ");
	scanf("%f", &op2);

	printf("Operator? [+, -, *, /] ");
	scanf(" %c", &operator);

	snprintf(req, sizeof(req), "[%f, %f, %c]", op1, op2, operator);

	send(sock, req, strlen(req), 0);

	recv(sock, res, sizeof(res), 0);
	printf("Server response: %s\n", res);

	return 0;
}

