#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define MIDDLEMAN_PORT 3333
#define BUFFER_SIZE 1024
<<<<<<< HEAD
#define CORRUPTION_PROBABILITY 10

void corrupt_message(char *msg, int length) {
    if (length <= 1) return;
=======
#define CORRUPTION_PROBABILITY 10  

void corrupt_message(char *msg, int length) {
    if (length <= 1) return; 
>>>>>>> origin/master

    int pos = rand() % (length - 1);

    msg[pos] ^= 0x01;  // XOR with 00000001

    printf("Middleman corrupted byte at position %d (changed to 0x%02X)\n", pos, (unsigned char)msg[pos]);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [server ip] [server port]\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in middleaddr, servaddr, cliaddr;
    char buffer[BUFFER_SIZE];

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    memset(&middleaddr, 0, sizeof(middleaddr));
    middleaddr.sin_family = AF_INET;
    middleaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    middleaddr.sin_port = htons(MIDDLEMAN_PORT);

    if (bind(sockfd, (const struct sockaddr *)&middleaddr, sizeof(middleaddr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    printf("Middleman running on port %d (10%% corruption chance)...\n", MIDDLEMAN_PORT);

    while (1) {
        socklen_t addr_len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                        (struct sockaddr *)&cliaddr, &addr_len);

        if (rand() % 100 < CORRUPTION_PROBABILITY) {
            corrupt_message(buffer, n);
        }

        sendto(sockfd, buffer, n, 0,
              (const struct sockaddr *)&servaddr, sizeof(servaddr));

        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                    (struct sockaddr *)&servaddr, &addr_len);
        sendto(sockfd, buffer, n, 0,
              (const struct sockaddr *)&cliaddr, addr_len);
    }

    close(sockfd);
    return 0;
}
