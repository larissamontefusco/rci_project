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

fd_set* master_set; // ponteiro para o master_fds
int max_fd;

/*
 * processa_comandos - Processa comandos digitados pelo usuário e executa as ações correspondentes.
 *
 * Parâmetros:
 *   buffer - String contendo o comando digitado pelo usuário.
 *   no - Estrutura contendo informações do nó atual.
 *
 * A função utiliza parse_buffer() para dividir o comando em palavras e
 * verifica qual comando foi inserido para executar a ação apropriada.
 */

int processa_comandos(char *buffer, int tamanho_buffer, INFO_NO *no) {
    system("clear");
    
    char words[10][100] = {""};
    parse_buffer(buffer, tamanho_buffer, words);
    
    if (strcmp(words[0], "join") == 0 || strcmp(words[0], "j") == 0) {
        printf("network = %s\n", words[1]);
        //join();
        return 0;
    } 
    else if (strcmp(words[0], "direct") == 0 && strcmp(words[1], "join") == 0) {
        printf("Network: %s\n", words[2]);
        printf("IP: %s\n", words[3]);
        printf("Porta: %s\n", words[4]);
        direct_join(words[2], *no, words[3], words[4], master_set, &max_fd);
        return 0;
    } 
    else if(strcmp("dj", words[0]) == 0){
        printf("Network: %s\n", words[1]);
        printf("IP: %s\n", words[2]);
        printf("Porta: %s\n", words[3]);
        direct_join(words[1], *no, words[2], words[3], master_set, &max_fd);
        return 0;
    }
    else if (strcmp(words[0], "create") == 0 || strcmp(words[0], "c") == 0) {
        printf("Comando Create\n");
        create(words[1], no);
        return 0;
    }
    else if (strcmp(words[0], "delete") == 0 || strcmp(words[0], "dl") == 0) {
        printf("Comando delete\n");
        return 0;
    }
    else if (strcmp(words[0], "retrieve") == 0 || strcmp(words[0], "r") == 0) {
        printf("Comando retrieve\n");
        return 0;
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "topology") == 0) {
        show_topology(*no);
        printf("show topology\n");
        return 0;
    }
    else if (strcmp(words[0], "st") == 0) {
        show_topology(*no);
        printf("show topology\n");
        return 0;
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "names") == 0) {
        printf("show names\n");
        return 0;
    }
    else if (strcmp(words[0], "sn") == 0) {
        printf("show names\n");
        return 0;
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "interest") == 0 && strcmp(words[2], "table") == 0) {
        printf("Comando show interest table\n");
        return 0;
    }
    else if (strcmp(words[0], "si") == 0) {
        printf("show interest table\n");
        return 0;
    }
    else if (strcmp(words[0], "leave") == 0 || strcmp(words[0], "l") == 0) {
        printf("Comando Leave\n");
        return 0;
    }
    else if (strcmp(words[0], "exit") == 0 || strcmp(words[0], "x") == 0) {
        printf("Comando Exit\n");
        return 1;
    }
    else {
        printf("Comando inválido! Por favor, tente novamente.\n");
        return 0;
    }    
}


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
    no.id.fd = SEM_CONEXAO;
    no.no_ext.fd = SEM_CONEXAO;
    no.no_salv.fd = SEM_CONEXAO;

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
    int finalizar_programa = 0;
    fd_set master_fds, read_fds;
    
    master_set = &master_fds;

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
    max_fd = max(fd, STDIN_FILENO);

    printf("========================================\n");
    printf("  Servidor iniciado com sucesso! 🎉\n");
    printf("  Ouvindo conexões na porta: %s\n", no.id.tcp);
    printf("========================================\n\n");
    
    printf("📌 COMANDOS DISPONÍVEIS:\n");
    printf("➡  direct join (dj) NET IP PORT  - Conectar a um nó diretamente\n");
    printf("➡  show topology (st)        - Exibir a topologia da rede\n");
    printf("➡  create (c) NAME          -  Criação de um objeto com nome NAME (tamanho máximo 100).\n\n");
    printf("➡  exit (x)                  - Sair do programa\n\n");
    
    printf("========================================\n");


    while (1) {
        if (finalizar_programa) {
            printf("Obrigado por utilizar nosso sistema! Até a próxima!\n");
            break;
        }
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
                    finalizar_programa = processa_comandos(buffer, TAMANHO_BUFFER, &no);
                } 
                else {
                    int n = read(i, buffer, TAMANHO_BUFFER);
                    if (n <= 0) {    
                        if (n == 0) {
                                printf("f_handle_disconnection(i) função, temos que fazer");
                            } else {
                                perror("read");
                            }
                            close(i);
                            FD_CLR(i, &master_fds); 
                        } 
                    else {
                        finalizar_programa = processa_comandos(buffer, TAMANHO_BUFFER, &no);
                    }
                }
            }
        }
    }
    return 0;
}
