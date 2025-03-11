#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#include "ndn_headers.h"



int testa_formato_porto(char *porto) {
    // Verifica se a string está vazia
    if (porto == NULL || *porto == '\0') {
        return 1; // Inválido
    }

    // Verifica se todos os caracteres são dígitos
    for (int i = 0; porto[i] != '\0'; i++) {
        if (!isdigit(porto[i])) {
            return 1; // Inválido
        }
    }

    // Converte para número e verifica intervalo
    int numero = atoi(porto);
    if (numero < 0 || numero > 65535) {
        return 1; // Inválido
    }

    return 0; // Válido
}

/** int testa_formato_ip
 * @brief Verifica se uma string está no formato correto de endereço IP (X.X.X.X).
 * X é um número entre 0 e 255.
 * @param ip String contendo o IP a ser validado.
 * @return int Retorna true (1) se for válido, false (0) caso contrário.
 */

int testa_formato_ip(char* ip) {
    if (ip == NULL) {
        return 1; // NULL não é um IP válido
    }

    int octetos = 0;  // Contador de octetos (partes do IP)
    int digit_count = 0;  // Contador de dígitos dentro de cada octeto
    int num = 0; // Armazena o valor do octeto

    for (int i = 0; ip[i] != '\0'; i++) {
        if (isdigit(ip[i])) {
            num = num * 10 + (ip[i] - '0');  // Constrói o número do octeto
            digit_count++;  

            if (num > 255) {
                printf("\nNúmero do octeto maior que 255: %c", ip[i]);
                return 1;
            }
        } 
        else if (ip[i] == '.') {
            if (digit_count == 0 || digit_count > 3) {
                printf("\nOcteto inválido (vazio ou com mais de 3 dígitos): %c", ip[i]);
                return 1; 
            }
            octetos++;  
            digit_count = 0;  
            num = 0;
        } 
        else {
            printf("\nCaractere inválido encontrado: %c", ip[i]);
            return 1;  
        }
    }

    // O último octeto deve ser válido e o IP deve ter exatamente 3 pontos
    if (digit_count == 0 || digit_count > 3 || octetos != 3) {
        return 1;
    }

    return 0; // IP válido
}

int join(char *rede_id, INFO_NO *no, char *regIP, char *regUDP) {
    // Isso aqui é UDP
    return 0;  
}

int direct_join(REDE *rede, INFO_NO *no, char *connectIP, char *connectTCP) {
    // Caso especial: Se connectIP for "0.0.0.0", cria a rede com apenas o nó
    if (strcmp(connectIP, "0.0.0.0") == 0) {
        rede->total_nos = 1;
        rede->nos_rede[0] = *no;  // Adiciona o nó à rede
        return 0; 
    }

    // Percorre a lista de nós na rede para encontrar um nó com o mesmo IP e porta TCP
    for (int i = 0; i < rede->total_nos; i++) {
        if (strcmp(rede->nos_rede[i].id.ip, connectIP) == 0 && 
            strcmp(rede->nos_rede[i].id.tcp, connectTCP) == 0) {
            return i; // Retorna o índice do nó encontrado
        }
    }

    return -1; // Nó não encontrado
}

/*
 * Função: parse_buffer
 * ---------------------
 * Divide o buffer lido e guarda cada palavra em um vetor de strings.
 *
 * buffer: String de entrada contendo palavras separadas por espaços.
 * words: Matriz de strings onde cada palavra será armazenada.
 */
void parse_buffer(char *buffer, int tamanho_buffer, char words[10][100]) {
    // Copia o vetor (porque usa strtok)
    char copia_buffer[tamanho_buffer]; 
    strncpy(copia_buffer, buffer, sizeof(copia_buffer) - 1);
    copia_buffer[sizeof(copia_buffer) - 1] = '\0';  
    
    // Usar strtok para dividir o buffer
    char *token = strtok(copia_buffer, " ");
    int index = 0;
    
    // Iterar pelos tokens e guardar no vetor words[][]
    while (token != NULL && index < 10) {
        // Caso a palavra termine com "\n", trocar por "\0"
        if (token[strlen(token) - 1] == '\n') {
            token[strlen(token) - 1] = '\0';
        }
        
        // Copiar para o vetor words[][]
        strncpy(words[index], token, 99);
        words[index][99] = '\0'; // Garantir terminação segura
        index++;
        token = strtok(NULL, " ");
    }
}

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

void processa_comandos(char *buffer, int tamanho_buffer, INFO_NO *no) {
    system("clear");
    
    char words[10][100] = {""};
    parse_buffer(buffer, tamanho_buffer, words);
    
    if (strcmp(words[0], "join") == 0 || strcmp(words[0], "j") == 0) {
        printf("network = %s\n", words[1]);
        //join();
    } 
    else if (strcmp(words[0], "direct") == 0 && strcmp(words[1], "join") == 0) {
        printf("Network: %s\n", words[2]);
        printf("IP: %s\n", words[3]);
        printf("Porta: %s\n", words[4]);
        //direct_join();
    } 
    else if(strcmp("dj", words[0]) == 0){
        printf("Network: %s\n", words[1]);
        printf("IP: %s\n", words[2]);
        printf("Porta: %s\n", words[3]);
        //direct_join();
    }
    else if (strcmp(words[0], "create") == 0 || strcmp(words[0], "c") == 0) {
        printf("Comando Create\n");
        create(words[1], no);
    }
    else if (strcmp(words[0], "delete") == 0 || strcmp(words[0], "dl") == 0) {
        printf("Comando delete\n");
    }
    else if (strcmp(words[0], "retrieve") == 0 || strcmp(words[0], "r") == 0) {
        printf("Comando retrieve\n");
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "topology") == 0) {
        printf("show topology\n");
    }
    else if (strcmp(words[0], "st") == 0) {
        printf("show topology\n");
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "names") == 0) {
        printf("show names\n");
    }
    else if (strcmp(words[0], "sn") == 0) {
        printf("show names\n");
    }
    else if (strcmp(words[0], "show") == 0 && strcmp(words[1], "interest") == 0 && strcmp(words[2], "table") == 0) {
        printf("Comando show interest table\n");
    }
    else if (strcmp(words[0], "si") == 0) {
        printf("show interest table\n");
    }
    else if (strcmp(words[0], "leave") == 0 || strcmp(words[0], "l") == 0) {
        printf("Comando Leave\n");
    }
    else if (strcmp(words[0], "exit") == 0 || strcmp(words[0], "x") == 0) {
        printf("Comando Exit\n");
    }
}


int create(char *name, INFO_NO *no) {
    if (strlen(name) >= tamanho_max_obj) {
        printf("Erro: Nome muito grande.\n");
        return 1;
    }

    // Encontrar um espaço livre no cache
    for (int i = 0; i < n_max_obj; i++) {
        if (no->cache[i][0] == '\0') { // Se a posição estiver vazia
            strcpy(no->cache[i], name);
            printf("Objeto '%s' armazenado na posição %d do cache.\n", name, i);
            return 0;
        }
    }

    printf("Erro: Cache cheio.\n");
    return 1; 
}