#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define MIDDLEMAN_PORT 2223
#define BUFFER_SIZE 1024
#define CORRUPTION_PROBABILITY 25

void corrupt_message(char *msg, int length) {
	if (length <= 1) return;  
	
	int r = rand();
	int pos = r % (length - 1);
	int bit_pos = r % 8;

	msg[pos] ^= 1 << bit_pos; 

	printf("Middleman corrupted %dth bit of byte at position %d (changed to %#02x).\n", (r%8)+1, pos, msg[pos]);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s [server ip] [server port]\n", argv[0]);
		return -1;
	}

	// Set random seed
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));

	// Create socket to listen for client
	int cli_sock;
	if ((cli_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Failed to create client socket");
		return -1;
	}
	
	// Configure middleman address
	struct sockaddr_in middleaddr = {0};
	middleaddr.sin_family = AF_INET;
	middleaddr.sin_addr.s_addr = INADDR_ANY;
	middleaddr.sin_port = htons(MIDDLEMAN_PORT);
	
	if (bind(cli_sock, (struct sockaddr*) &middleaddr, sizeof(middleaddr)) < 0) {
		perror("Failed to bind server socket");
		return -1;
	}
	printf("Middleman running on port %d (%d%% corruption chance)...\n", MIDDLEMAN_PORT, CORRUPTION_PROBABILITY);

	// Listen for connections
	// Backlog=1 to only accept one single connection
	if (listen(cli_sock, 1) != 0) {
		perror("Failed to listen for client");
		return -1;
	}

	// Accept client connection
	struct sockaddr_in cliaddr = {0};
	socklen_t cliaddr_len = sizeof(cliaddr);
	int connfd;
	if ((connfd = accept(cli_sock, (struct sockaddr*)&cliaddr, &cliaddr_len)) < 0) {
		perror("Failed to accept client connection");
		return -1;
	}
	
	// Create socket to retransmit to server
	int srv_sock;
	if ((srv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Failed to create server socket");
		return -1;
	}

	// Configure server address
	struct sockaddr_in servaddr = {0};
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if (connect(srv_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("Failed to connect to server");
	}

	// Hard-coded Hamming encoding for the string "exit", to know
	// when to shut down along with the client and server.
	const uint8_t exit_encoded[] = {
		0x2d, 0x33, 0x4b, 0x34, 0x4c, 0x33, 0x2a, 0x34, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x00
	};

	char msg[BUFFER_SIZE];
	size_t msg_len;
	int repeat = 1;
	while(repeat) {
		if ((msg_len = recv(connfd, msg, BUFFER_SIZE, 0)) < 0) {
			perror("Failed to receive from client");
			return -1;
		}

		if (msg_len == sizeof(exit_encoded))
			repeat = (memcmp(msg, exit_encoded, sizeof(exit_encoded)) != 0);

		// Corrupt (or not) message before retransmission
		if (rand() % 100 < CORRUPTION_PROBABILITY)
			corrupt_message(msg, msg_len);
		else
			printf("Retransmitting message without corruption.\n");
	
		if (send(srv_sock, msg, msg_len, 0) < 0) {
			perror("Failed to retransmit message to server");
			return -1;
		}
	
		// Receive server response
		if ((msg_len = recv(srv_sock, msg, BUFFER_SIZE, 0)) < 0) {
			perror("Failed to receive from server");
			return -1;
		}
	
		printf("Retransmitting to client.\n");
		// Retransmit response to client
		if (send(connfd, msg, msg_len, 0) < 0) {
			perror("Failed to retransmit message to client");
			return -1;
		}
	}
	
	close(srv_sock);
	close(cli_sock);
	return 0;
}
