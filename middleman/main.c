#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define MIDDLEMAN_PORT 2222
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
		exit(1);
	}

	// Set random seed
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));

	// Create sockets
	int srv_sock, cli_sock;
	if ((srv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Server socket creation failed");
		return -1;
	}
	if ((cli_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Client socket creation failed");
		return -1;
	}
	
	struct sockaddr_in middleaddr = {0}, servaddr = {0}, cliaddr = {0};
	char buffer[BUFFER_SIZE];

	// Configure middleman address
	middleaddr.sin_family = AF_INET;
	middleaddr.sin_addr.s_addr = INADDR_ANY;
	middleaddr.sin_port = htons(MIDDLEMAN_PORT);

	// Bind server socket to port
	// Binding the client socket is unnecessary
	if (bind(srv_sock, (struct sockaddr*) &middleaddr, sizeof(middleaddr)) < 0) {
		perror("Failed to bind server socket");
		exit(1);
	}

	// Configure server address
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	printf("Middleman running on port %d (%d%% corruption chance)...\n", MIDDLEMAN_PORT, CORRUPTION_PROBABILITY);

	socklen_t addr_len = sizeof(cliaddr);
	int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
					(struct sockaddr *)&cliaddr, &addr_len);

	if (rand() % 100 < CORRUPTION_PROBABILITY) {
		corrupt_message(buffer, n);
	} else {
		printf("Middleman retrasmitted message without any corruption.\n");
	}

	sendto(sockfd, buffer, n, 0,
			(const struct sockaddr *)&servaddr, sizeof(servaddr));

	n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
			(struct sockaddr *)&servaddr, &addr_len);
	sendto(sockfd, buffer, n, 0,
			(const struct sockaddr *)&cliaddr, addr_len);

	close(sockfd);
	return 0;
}
