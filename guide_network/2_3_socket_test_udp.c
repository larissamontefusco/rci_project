#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 128

int main(void) {
    int fd;
    struct sockaddr addr;
    socklen_t addrlen;
    ssize_t n;
    char buffer[BUFFER_SIZE + 1];

    // Tamanho da estrutura de endereço
    addrlen = sizeof(addr);

    // Recebe dados do socket
    n = recvfrom(fd, buffer, BUFFER_SIZE, 0, &addr, &addrlen);
    if (n == -1) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0'; // Garante a terminação da string
    printf("Echo: %s\n", buffer);

    close(fd);
    return 0;
}