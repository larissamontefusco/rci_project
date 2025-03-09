/******************************************************************************
 * Redes de Comunicação e Internet
 * LEEC 24/25
 *
 * Projecto
 * ndn.c
 * 
 * Por: Larissa da Silva Montefusco e Pedro Henrique Lucas Gouveia
 *           
 *****************************************************************************/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>

#include "ndn_headers.h"
#define max(A,B) ((A) >= (B) ? (A) : (B))

int main(int argc, char** argv)
{   
    
    INFO_NO no;
    char regIP[tamanho_ip]="193.136.138.142", regUDP[tamanho_porto]="59000";
    int error = testa_formato_ip(argv[2]);
    if (error){
        return 0;
    }
    switch (argc)
    {   
        case 4 : // regista o id do nó
                strcpy(no.id.ip, argv[2]);
                strcpy(no.id.tcp, argv[3]);
                break;
        case 6 : //regista regIP e regUDP
                strcpy(no.id.ip, argv[2]);
                strcpy(no.id.tcp, argv[3]);
                strcpy(regIP, argv[4]);
                strcpy(regUDP, argv[5]);
                break;
        default :   
                printf("Erro na invocação da aplicação");
                exit(0);
    }
    
    struct addrinfo hints, *res;
    int errcode;
    int fd, newfd, afd = 0;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;
    char buffer[128];

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
    FD_ZERO(&rfds);
    while (1) {
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
        
        // Mask: rfds
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
                        write(newfd, "busy\n", 6);
                        close(newfd);
                    } 
                    else if (FD_ISSET(afd, &rfds)) {
                        FD_CLR(afd, &rfds);
                        if ((n = read(afd, buffer, 128)) != 0) {
                            if (n == -1) exit(1); // error
                            write(afd, buffer, n);
                        } 
                        else {
                            close(afd);
                            state = idle; // connection closed by peer
                        }
                    }
                    break;
            }
        }
    }
    return 0;

}