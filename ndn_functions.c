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
 * @brief Verifica se uma string está no formato correto de endereço IP (XXX.XXX.XXX.XXX).
 * XXX é um número entre 0 e 255.
 * @param ip String contendo o IP a ser validado.
 * @return int Retorna true (1) se for válido, false (0) caso contrário.
 */

 int testa_formato_ip(char* ip) {
    int octetos = 0;  // Contador de octetos (partes do IP separados por ponto)
    int digit_count = 0;  // Contador de dígitos dentro de cada octeto

    for (int i = 0; ip[i] != '\0'; i++) {
        if (isdigit(ip[i])) {
            digit_count++;  // Conta os dígitos
        } 
        else if (ip[i] == '.') {
            octetos++;  // Conta os pontos (separadores de octetos)
            if (digit_count < 1 || digit_count > 3) {
                return 1;  
            }
            digit_count = 0;  // Reinicia o contador de dígitos para o próximo octeto
        } 
        else {
#if DEBUG
        printf("\nCaractere inválido encontrado");
#endif
            return 1;  // Se encontrar caractere inválido
        }
    }
    // O último octeto deve ter de 1 a 3 dígitos
    if (digit_count < 1 || digit_count > 3) {
        return 1;  // Se o último octeto não tiver entre 1 e 3 dígitos
    }

    return 0;
}
