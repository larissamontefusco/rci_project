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
    error = testa_formato_porto(argv[3]);
    
    printf("Error = %d\n", error);

    if (error) {
        printf("Formato de inputs inválido!\n");
        return 1;
    }

    switch (argc) {   
        case 4:
            strcpy(no.id.ip, argv[2]);
            strcpy(no.id.tcp, argv[3]);
            printf("Case 4\n");
            break;
        case 6:
            strcpy(no.id.ip, argv[2]);
            strcpy(no.id.tcp, argv[3]);
            strcpy(regIP, argv[4]);
            strcpy(regUDP, argv[5]);
            error = testa_formato_ip(argv[4]);
            error = testa_formato_porto(argv[5]);
            if (error) {
                printf("Formato de inputs inválido!\n");
                return 1;
            }
            printf("Case 6\n");
            break;
        default:
            printf("Erro na invocação da aplicação");
            exit(0);
    }

    struct addrinfo hints, *res;
    int fd, newfd;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;
    char buffer[128];
    
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit(1);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, argv[3], &hints, &res) != 0) exit(1);
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) exit(1);
    if (listen(fd, 5) == -1) exit(1);
    
    while (1) {
        addrlen = sizeof(addr);
        if ((newfd = accept(fd, &addr, &addrlen)) == -1) exit(1);
        
        while ((n = read(newfd, buffer, 128)) > 0) {
            write(newfd, buffer, n);
            // JOIN:
            if (strncmp(buffer, "join ", 5) == 0) {
                char *network = buffer + 5;
                printf("network = %s\n", network);
                //join();
            } else if (strncmp(buffer, "j ", 2) == 0) {
                char *network = buffer + 2;
                printf("network = %s\n", network);
                //join();
            } else if (strncmp(buffer, "create ", 7) == 0 || strncmp(buffer, "c ", 2) == 0) {
                printf("Comando Create\n");
                char *name = buffer + (buffer[0] == 'c' ? 2 : 7);
                create(name, &no);
            } else if (strncmp(buffer, "delete", 6) == 0 || strncmp(buffer, "dl", 3) == 0) {
                printf("Comando delete\n");
            } else if (strncmp(buffer, "retrieve", 6) == 0 || strncmp(buffer, "r ", 2) == 0) {
                printf("Comando retrieve\n");
            } else if (strncmp(buffer, "show topology", 13) == 0 || strncmp(buffer, "st ", 3) == 0) {
                printf("show topology\n");
            } else if (strncmp(buffer, "show names", 10) == 0 || strncmp(buffer, "sn ", 3) == 0) {
                printf("show names\n");
            } else if (strncmp(buffer, "show interest table", 19) == 0 || strncmp(buffer, "si ", 3) == 0) {
                printf("Comando show interest table\n");
            } else if (strncmp(buffer, "leave", 5) == 0 || strncmp(buffer, "l ", 2) == 0) {
                printf("Comando Leave\n");
            } else if (strncmp(buffer, "exit", 4) == 0 || strncmp(buffer, "x ", 2) == 0) {
                printf("Comando Exit\n");
            }
        }
        close(newfd);
    }
    return 0;
}
