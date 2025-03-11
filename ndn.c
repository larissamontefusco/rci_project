#define _POSIX_C_SOURCE 200112L
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
#include <arpa/inet.h>

#include "ndn_headers.h"

#define max(A,B) ((A) >= (B) ? (A) : (B))
#define TAMANHO_BUFFER 128

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
    
    if (error) {
        printf("Formato de inputs inválido!\n");
        return 1;
    }

    strcpy(no.id.ip, argv[2]);
    strcpy(no.id.tcp, argv[3]);
    if (argc == 6) {
        strcpy(regIP, argv[4]);
        strcpy(regUDP, argv[5]);
        error = testa_formato_ip(argv[4]);
        error = testa_formato_porto(argv[5]);
        if (error) {
            printf("Formato de inputs inválido!\n");
            return 1;
        }
    }

    struct addrinfo hints, *res;
    int fd, new_fd, counter;
    fd_set master_fds, read_fds;
    struct sockaddr addr;
    socklen_t addrlen;
    char buffer[TAMANHO_BUFFER];
    
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit(1);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, no.id.tcp, &hints, &res) != 0) exit(1);
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) exit(1);
    if (listen(fd, 5) == -1) exit(1);
    
    FD_ZERO(&master_fds);
    FD_SET(fd, &master_fds);
    FD_SET(STDIN_FILENO, &master_fds);
    int max_fd = max(fd, STDIN_FILENO);

    printf("Servidor ouvindo na porta %s...\n", no.id.tcp);
    while (1) {
        
        read_fds = master_fds;
        
                
        counter = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (counter == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= max_fd; i++) {
            memset(buffer, 0, TAMANHO_BUFFER);  // Clear the buffer before reading new data 

            if (FD_ISSET(i, &read_fds)) { 
                if (i == fd) {              
                    addrlen = sizeof addr;
                    new_fd = accept(fd, &addr, &addrlen);
                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(new_fd, &master_fds);
                        if (new_fd > max_fd) max_fd = new_fd;
                    }
                }
                else if (i == STDIN_FILENO) {
                    fgets(buffer, TAMANHO_BUFFER, stdin);
                    processa_comandos(buffer, TAMANHO_BUFFER, &no);
                } 
                else {
                    int n = read(i, buffer, TAMANHO_BUFFER);
                    if (n <= 0) {    
                        if (n == 0) {
                                printf("f_handle_disconnection(i)");
                            } else {
                                perror("read");
                            }
                            close(i);
                            FD_CLR(i, &master_fds);     // Remove closed connection from master set
                        } 
                    else {
                        processa_comandos(buffer, TAMANHO_BUFFER, &no);
                    }
                }
            }
        }
    }
    return 0;
}
