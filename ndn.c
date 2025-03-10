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

int main(int argc, char** argv) {   
    if (argc != 4 && argc != 6) {
        return 1;
    }

    INFO_NO no;
    REDE rede_;
    strcpy(rede_.id, "020");
    
    char regIP[tamanho_ip] = "193.136.138.142";
    char regUDP[tamanho_porto] = "59000";

    int error = testa_formato_ip(argv[2]);
    error = testa_formato_ip(argv[4]);

    printf("Error = %d\n", error); // Confirma que error está sendo calculado

    if (error) {
        printf("Formato de IP inválido!\n");
        return 1;
    }

    switch (argc)
    {   
        case 4 : // regista o id do nó
                strcpy(no.id.ip, argv[2]);
                strcpy(no.id.tcp, argv[3]);
                printf("Case 4\n");
                break;
        case 6 : //regista regIP e regUDP
                strcpy(no.id.ip, argv[2]);
                strcpy(no.id.tcp, argv[3]);
                strcpy(regIP, argv[4]);
                strcpy(regUDP, argv[5]);
                printf("Case 6\n");
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

    if ((errcode = getaddrinfo(NULL, argv[3], &hints, &res)) != 0) exit(1); // error
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
                            if (strncmp(buffer, "join ", 5) == 0) {
                                char *network = buffer + 5;
                                printf("network = %s\n", network);
                                //join();
                            }
                            else if (strncmp(buffer, "j ", 2) == 0) {
                                char *network = buffer + 2;
                                printf("network = %s\n", network);
                                //join();  
                            }
                            else if (strncmp(buffer, "create ", 7) == 0 || strncmp(buffer, "c ", 2) == 0) {
                                printf("Comando Create\n");
                                char *name = buffer + (buffer[0] == 'c' ? 2 : 7);  // Pega a parte depois de "create " ou "c "
                                create(name, &no);
                            } 
                            else if (strncmp(buffer, "delete", 6) == 0 || strncmp(buffer, "dl ", 2) == 0) {
                                printf("Comando delete\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "retrieve", 6) == 0 || strncmp(buffer, "r ", 2) == 0) {
                                printf("Comando retrieve\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "show topology", 6) == 0 || strncmp(buffer, "st ", 2) == 0) {
                                printf("show topology\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "show names", 6) == 0 || strncmp(buffer, "sn ", 2) == 0) {
                                printf("show names\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "show interest table", 6) == 0 || strncmp(buffer, "si ", 2) == 0) {
                                printf("Comando show interest table\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "leave", 6) == 0 || strncmp(buffer, "l ", 2) == 0) {
                                printf("Comando Leave\n");
                                // Chamar a função correspondente
                            } 
                            else if (strncmp(buffer, "exit", 6) == 0 || strncmp(buffer, "x ", 2) == 0) {
                                printf("Comando Exit\n");
                                // Chamar a função correspondente
                            } 
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