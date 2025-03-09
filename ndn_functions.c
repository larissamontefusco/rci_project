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
 * @brief Testa o número de argumentos passados para a aplicação.
 * 
 * @param argc Número de argumentos.
 * @return int Retorna 4 se forem 4 argumentos, 6 se forem 6 argumentos, ou 0 se houver erro.
 */
int testa_numero_argumentos(int argc)
{
#if DEBUG
    printf("\nFunção testa_numero_argumentos. argc = %d", argc);
#endif
    if (argc == 4) {
        return 4;
    } else if (argc == 6) {
        return 6;
    }
    return 0; // Retorna 0 em caso de erro no número de argumentos
}

/**
 * @brief Verifica se uma string está no formato correto de endereço IP (XXX.XXX.XXX.XXX).
 * 
 * @param ip String contendo o IP a ser validado.
 * @return int Retorna true (1) se for válido, false (0) caso contrário.
 */

 int testa_formato_ip(char* ip) {
    int octetos = 0;  // Contador de octetos (partes do IP separados por ponto)
    int digit_count = 0;  // Contador de dígitos dentro de cada octeto
    int primeiro_octo = 1; // Flag para o primeiro octeto

    // Verifica se o comprimento do IP é válido (mínimo 13, máximo 15 caracteres)
    if (!(strlen(ip) >= 9 && strlen(ip) <= 15)) {
#if DEBUG
        printf("\nO tamanho do IP está inválido");
#endif
        return 0;  // false, 0 em C
    }

    for (int i = 0; ip[i] != '\0'; i++) {
        if (isdigit(ip[i])) {
            digit_count++;  // Conta os dígitos
        } else if (ip[i] == '.') {
            octetos++;  // Conta os pontos (separadores de octetos)
            
            // Verifica se o número de dígitos no octeto é válido
            if (primeiro_octo) {
                if (digit_count != 3) {
                    return 0;  // O primeiro octeto deve ter exatamente 3 dígitos
                }
                primeiro_octo = 0;  // Após o primeiro octeto, a flag é desmarcada
            } else {
                if (digit_count < 1 || digit_count > 3) {
                    return 0;  // Os outros octetos devem ter de 1 a 3 dígitos
                }
            }
            
            digit_count = 0;  // Reinicia o contador de dígitos para o próximo octeto
        } else {
#if DEBUG
        printf("\nCaractere inválido encontrado");
#endif
            return 0;  // Se encontrar caractere inválido
        }
    }

    // Verifica o último octeto após o loop
    if (primeiro_octo) {
        return 0;  // Caso o IP não tenha 4 octetos
    }

    // O último octeto deve ter de 1 a 3 dígitos
    if (digit_count < 1 || digit_count > 3) {
        return 0;  // Se o último octeto não tiver entre 1 e 3 dígitos
    }

    // O IP deve conter exatamente 3 pontos (o que significa 4 octetos)
    return (octetos == 3);  // Se houver exatamente 3 pontos, são 4 octetos
}

/**
 * @brief Detecta erros na invocação da aplicação.
 * 
 * @param argc Número de argumentos.
 * @param argv Array de strings contendo os argumentos.
 * @return int Retorna 4 ou 6 se estiver correto, ou 0 se houver erro.
 */
int deteta_erro_invocacao(int argc, char** argv) {
    int n_arg = 0;
    n_arg = testa_numero_argumentos(argc);

    if (!n_arg) {
        return 0; // Erro no número de argumentos
    }

    // Testa o nome da aplicação
    if (strcmp(argv[0], "./ndn") != 0) {
        return 0;
    }

    // Testa se o IP está no formato correto
    if (!testa_formato_ip(argv[2])) {
        return 0;
    }

    // Testa se a porta TCP é válida (deve estar entre 1 e 99999)
    if (!(atoi(argv[3]) > 0 && atoi(argv[3]) < 100000)) {
        return 0;
    }

    if (n_arg == 6) {
        // Testa se regIP está no formato correto
        if (!testa_formato_ip(argv[4])) {
            return 0;
        }

        // Testa se a porta regUDP é válida (deve estar entre 1000 e 99999)
        if (!(atoi(argv[5]) > 999 && atoi(argv[5]) < 100000)) {
            return 0;
        }
    }

    return n_arg;
}