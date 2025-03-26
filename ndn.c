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
char regIP[tamanho_ip] = "193.136.138.142";
char regUDP[tamanho_porto] = "59000";

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

int processa_comandos(int fd, char *buffer, int tamanho_buffer, INFO_NO *no) {
    system("clear");
    
    char words[10][100] = {""};
    parse_buffer(buffer, tamanho_buffer, words);
    if (fd == STDIN_FILENO) {
        if (strcmp(words[0], "join") == 0 || strcmp(words[0], "j") == 0) {
            printf("network = %s\n", words[1]);
            join(words[1], no, regIP, regUDP, master_set, &max_fd);
            printf("Opa, to aqui do lado de fora! no->net.net_id = %s",no->net.net_id);
            return 0;
        } 
        else if (strcmp(words[0], "direct") == 0 && strcmp(words[1], "join") == 0) {
            direct_join(no, words[2], words[3], master_set, &max_fd);
            return 0;
        } 
        else if(strcmp("dj", words[0]) == 0){
            direct_join(no, words[1], words[2], master_set, &max_fd);
            return 0;
        }
        else if (strcmp(words[0], "create") == 0 || strcmp(words[0], "c") == 0) {
            create(words[1], no);
            return 0;
        }
        else if (strcmp(words[0], "delete") == 0 || strcmp(words[0], "dl") == 0) {
            delete(words[1], no);
            return 0;
        }
        else if (strcmp(words[0], "retrieve") == 0 || strcmp(words[0], "r") == 0) {
            retrieve(words[1], no);
            return 0;
        }
        else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "topology") == 0) {
            show_topology(no);
            return 0;
        }
        else if (strcmp(words[0], "st") == 0) {
            show_topology(no);
            return 0;
        }
        else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "names") == 0) {
            show_names(no);
            return 0;
        }
        else if (strcmp(words[0], "sn") == 0) {
            show_names(no);
            return 0;
        }
        else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "interest") == 0 && strcmp(words[2], "table") == 0) {
            show_interest_table(no);
            return 0;
        }
        else if (strcmp(words[0], "si") == 0) {
            show_interest_table(no);
            printf("show interest table\n");
            return 0;
        }
        else if (strcmp(words[0], "leave") == 0 || strcmp(words[0], "l") == 0) {
            leave(no);
            return 0;
        }
        else if (strcmp(words[0], "exit") == 0 || strcmp(words[0], "x") == 0) {
            return 1;
        }
    } else {
        if(strcmp("ENTRY", words[0]) == 0){
            recebendo_entry(no, fd, words[1],words[2]);
            return 0;
        }
        else if(strcmp("SAFE", words[0]) == 0){
            recebendo_safe(no, fd, words[1],words[2]);
            return 0;
        }
        else if(strcmp("INTEREST", words[0]) == 0){
            int origem_interface = -1;
        
            // Encontrar a interface correspondente ao `fd`
            for (int i = 0; i < n_max_internos; i++) {
                if (no->no_int[i].fd == fd) {
                    origem_interface = i;
                    break;
                }
            }
        
            if (origem_interface == -1) {
                printf("⚠️ Erro: Interface de origem não encontrada para fd=%d\n", fd);
                return 0;
            }
        
            recebendo_interesse(no, words[1], origem_interface);
            return 0;
        }        
        else{
            printf("Comando não encontrado ❌. Tente novamente.\n");
            return 0;
        }
    }  
    return 0;
}

int n_max_obj;

int main(int argc, char** argv) {   
    // Testa se os parametros dados na linha de comando estão correctos
    if (testa_invocacao_programa(argc,argv)) exit(1);
    
    n_max_obj = atoi(argv[1]); // Converte argv[1] para inteiro

    if (n_max_obj <= 0 || n_max_obj > 1000) { 
        fprintf(stderr, "Erro: <n_max_obj> deve estar entre 1 e 1000.\n");
        return 1;
    }

    INFO_NO no;
    inicializar_no(&no);

    strcpy(no.id.ip, argv[2]);
    strcpy(no.id.tcp, argv[3]);

    if (argc == 6) {
        strcpy(regIP, argv[4]);
        strcpy(regUDP, argv[5]);
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
    printf("➡  direct join (dj) IP PORT  - Conectar a um nó diretamente\n");
    printf("➡  join (j) NET - Entrada do nó à rede net\n");
    printf("➡  show topology (st)        - Exibir a topologia da rede\n");
    printf("➡  create (c) NAME          -  Criação de um objeto com nome NAME (tamanho máximo 100).\n");
    printf("➡  retrieve (r) NAME          -  Pesquisa do objeto com nome NAME (tamanho máximo 100).\n");
    printf("➡  delete (dl) NAME          -  Exclusão de um objeto com nome NAME (tamanho máximo 100).\n");
    printf("➡  show names (sn)          -  Visualização dos nomes de todos os objetos guardados no nó.\n");
    printf("➡  show interest table (si)   -  Visualização de todas as entradas da tabela de interesses pendentes.\n\n");
    printf("➡  leave (l)                  - Saída do nó da rede.\n\n");
    printf("➡  exit (x)                  - Sair do programa\n\n");
    
    printf("========================================\n");
    
   

    while (1) {
        if (finalizar_programa) {
            printf("Obrigado por utilizar nosso sistema! Até a próxima! 😃\n");
            exit(0);
        }
        read_fds = master_fds;
        
        // Definindo um timeout para o select
        struct timeval timeout;
        timeout.tv_sec = 5;  // Segundos
        timeout.tv_usec = 0; // Microssegundos

                
        counter = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
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
                    finalizar_programa = processa_comandos(i, buffer, TAMANHO_BUFFER, &no);
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
                        finalizar_programa = processa_comandos(i, buffer, TAMANHO_BUFFER, &no);
                    }
                }
            }
        }
    }
    return 0;
}
