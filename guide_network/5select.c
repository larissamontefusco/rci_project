#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

int main(void) {
    int fd, newfd, afd = 0;
    fd_set rfds;
    enum { idle, busy } state;
    int maxfd, counter;
    struct sockaddr_in addr;
    socklen_t addrlen;
    char buffer[128];

    fd = socket(AF_INET, SOCK_STREAM, 0);
    // Configuração do socket omitida...
    listen(fd, 5);
    state = idle;
    
    while (1) {
        FD_ZERO(&rfds);
        switch (state) {
            case idle:
                FD_SET(fd, &rfds);
                maxfd = fd;
                break;
            case busy:
                FD_SET(fd, &rfds);
                if (afd > 0) {
                    FD_SET(afd, &rfds);
                    maxfd = MAX(fd, afd);
                } else {
                    maxfd = fd;
                }
                break;
        }
        
        counter = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (counter < 0) {
            perror("select");
            exit(1);
        }
        
        for (; counter > 0; counter--) {
            switch (state) {
                case idle:
                    if (FD_ISSET(fd, &rfds)) {
                        FD_CLR(fd, &rfds);
                        addrlen = sizeof(addr);
                        afd = accept(fd, (struct sockaddr*)&addr, &addrlen);
                        if (newfd < 0) {
                            perror("accept");
                            exit(1);
                        }
                        state = busy;
                    }
                    break;
                case busy:
                    if (FD_ISSET(fd, &rfds)) {
                        FD_CLR(fd, &rfds);
                        addrlen = sizeof(addr);
                        newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
                        if (newfd < 0) {
                            perror("accept");
                            exit(1);
                        }
                        // Envia "busy\n" para o novo cliente
                        write(newfd, "busy\n", 6);
                        close(newfd);
                    }
                    if (afd > 0 && FD_ISSET(afd, &rfds)) {
                        FD_CLR(afd, &rfds);
                        int n = read(afd, buffer, 128);
                        if (n <= 0) {
                            close(afd);
                            state = idle;
                        } else {
                            write(afd, buffer, n);
                        }
                    }
                    break;
            }
        }
    }
    return 0;
}
