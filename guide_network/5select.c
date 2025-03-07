#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define max(A,B) ((A) >= (B) ? (A) : (B))

int main(void) {
    struct addrinfo hints, *res;
    int errcode;
    int fd, newfd, afd = 0;
    ssize_t n, nw;
    struct sockaddr addr;
    socklen_t addrlen;
    char *ptr, buffer[128];

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit(1); // error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;

    if ((errcode = getaddrinfo(NULL, "58001", &hints, &res)) != 0) exit(1); // error
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) exit(1); // error
    if (listen(fd, 5) == -1) exit(1); // error

    fd_set rfds;
    enum { idle, busy } state;
    int maxfd, counter;

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
                FD_SET(afd, &rfds);
                maxfd = max(fd, afd);
                break;
        }

        counter = select(maxfd + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
        if (counter <= 0) exit(1); // error

        for (; counter; --counter) {
            switch (state) {
                case idle:
                    if (FD_ISSET(fd, &rfds)) {
                        FD_CLR(fd, &rfds);
                        addrlen = sizeof(addr);
                        if ((newfd = accept(fd, &addr, &addrlen)) == -1) exit(1); // error
                        afd = newfd;
                        state = busy;
                    }
                    break;
                
                case busy:
                    if (FD_ISSET(fd, &rfds)) {
                        FD_CLR(fd, &rfds);
                        addrlen = sizeof(addr);
                        if ((newfd = accept(fd, &addr, &addrlen)) == -1) exit(1); // error
                        /* ... write "busy\n" in newfd */
                        close(newfd);
                    } else if (FD_ISSET(afd, &rfds)) {
                        FD_CLR(afd, &rfds);
                        if ((n = read(afd, buffer, 128)) != 0) {
                            if (n == -1) exit(1); // error
                            /* ... write buffer in afd */
                        } else {
                            close(afd);
                            state = idle; // connection closed by peer
                        }
                    }
                    break;
            }
        }
    }
}
