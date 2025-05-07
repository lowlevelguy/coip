#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
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
	uint16_t server_port = 0;
	for (int i = 0; i < strlen(argv[2]); i++) {
		if (!isdigit(argv[2][i]) || server_port+argv[2][i] >= 6553 + 5) {
			fprintf(stderr, "Please provide a valid port number.");
			return -1;
		}
		server_port *= 10;
		server_port += argv[2][i];
	}
	server_port = htons(server_port);	

	int sock = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
	if (sock != -1) {
		perror("Could not create socket: ");
		return -1;
	}

	struct sockaddr_in local_addr = {0};
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(2222);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, &local_addr, sizeof(local_addr)) != 0) {
		perror("Could not bind socket to local address on port 4444: ");
		return -1;
	}

	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = server_port;
	if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) == 0) {
		fprintf(stderr, "Please provide a valid IP address.");
		return -1;
	}

	if (connect(sock, &server_addr, sizeof(server_addr)) != 0) {
		perror("Could not connect to server: ");
		return -1;
	}

	return 0;
}

