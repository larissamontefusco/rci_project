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

    // Verifica se o comprimento do IP é válido (mínimo 13, máximo 15 caracteres)
    if (!(strlen(ip) >= 13 && strlen(ip) <= 15)) {
        return false;
    }

    for (int i = 0; ip[i] != '\0'; i++) {
        if (isdigit(ip[i])) {
            digit_count++;
        } else if (ip[i] == '.') {
            octetos++;
            if (digit_count == 0 || digit_count > 3) {
                return false;  // Se não houve dígitos ou mais de 3 por octeto
            }
            digit_count = 0;  // Reinicia contador para próximo octeto
        } else {
            return false;  // Se encontrar caractere inválido
        }
    }

    // Último octeto deve ter de 1 a 3 dígitos
    if (digit_count == 0 || digit_count > 3) {
        return false;
    }

    // O IP deve conter exatamente 3 pontos
    return (octetos == 3);
}

/**
 * @brief Detecta erros na invocação da aplicação.
 * 
 * @param argc Número de argumentos.
 * @param argv Array de strings contendo os argumentos.
 * @return int Retorna 4 ou 6 se estiver correto, ou 0 se houver erro.
 */
int deteta_erro_invocacao(int argc, char** argv) {
    int n_arg = testa_numero_argumentos(argc);
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