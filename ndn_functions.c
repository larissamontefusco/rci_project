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

/**
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

int join(REDE *rede, INFO_NO *no) {
    // Verifica se ainda há espaço na rede
    if (rede->total_nos >= n_max_nos) {
        printf("Erro: Rede cheia! Não é possível adicionar mais nós.\n");
        return 1;  // Falha ao adicionar
    }

    // Adiciona o nó à rede
    rede->nos_rede[rede->total_nos] = *no;

    // Incrementa o contador de nós
    rede->total_nos++;

    printf("Nó adicionado à rede: IP=%s, TCP=%s\n", no->id.ip, no->id.tcp);
    return 0;  
}

int direct_join(){

    return 0;
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