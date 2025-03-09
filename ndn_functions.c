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
#if DEBUG
                printf("\nNúmero do octeto maior que 255: %c", ip[i]);
#endif
                return 1;
            }
        } 
        else if (ip[i] == '.') {
            if (digit_count == 0 || digit_count > 3) {
#if DEBUG
                printf("\nOcteto inválido (vazio ou com mais de 3 dígitos): %c", ip[i]);
#endif
                return 1; 
            }
            octetos++;  
            digit_count = 0;  
            num = 0;
        } 
        else {
#if DEBUG
            printf("\nCaractere inválido encontrado: %c", ip[i]);
#endif
            return 1;  
        }
    }

    // O último octeto deve ser válido e o IP deve ter exatamente 3 pontos
    if (digit_count == 0 || digit_count > 3 || octetos != 3) {
        return 1;
    }

    return 0; // IP válido
}
