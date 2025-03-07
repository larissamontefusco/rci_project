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

int testa_numero_argumentos(int argc)
{
    /*Testa o número de argumentos*/
    if (argc==4) /*Se houver apenas 4 argumentos (sem regIP e regUDP)*/
    {
        return 4; /*Retorna o numero de argumentos (4)*/
    }
    else
    {   
        if (argc==6) /*Se não houver 4, testa se são 6 (com regIP e regUDP)*/
        {
            return 6; /*Retorna o numero de argumentos (4)*/
        }
    }
    return 0; /* Retorna 0 em caso de ERRO no nº de argumentos */
}

int testa_formato_ip(char* ip) {
    int octetos = 0;  // Contador de octetos (partes do IP separados por ponto)
    int digit_count = 0;  // Contador de dígitos dentro de cada octeto
printf("\n\t1111111\n");
    // Verifica se o comprimento do IP é exatamente 15 caracteres (XXX.XXX.XXX.XXX)
    printf("strlen--->%lu", strlen(ip));
    if ( !(strlen(ip) >=13 && strlen(ip)<= 15) ) {
        return false;
    }
printf("/n/t22222222/n");
    // Verifica cada caractere da string
    for (int i = 0; ip[i] != '\0'; i++) {
        if (isdigit(ip[i])) {
            digit_count++;
        } else if (ip[i] == '.') {
            // Se encontrou um ponto, incrementa o contador de octetos
            octetos++;
            if (digit_count == 0 || digit_count > 3) {
                printf("/n/t333333333/n");
                return false;  // Se não houve dígitos ou mais de 3 dígitos por octeto
            }
            digit_count = 0;  // Reinicia o contador de dígitos para o próximo octeto
        } else {printf("/n/t444444444/n");
            return false;  // Se encontrar um caractere inválido
        }
    }

    // Depois do último ponto, ainda deve haver 1 a 3 dígitos (o último octeto)
    if (digit_count == 0 || digit_count > 3) {
        return false;
    }

    // O IP deve ter 3 pontos (4 octetos)
    if (octetos != 3) {
        return false;
    }

    return true;
}



/* Função que deteta erro na invocação da aplicação (retorna n_arg=0 se houver erro ou n_arg=4 ou 6)*/
int deteta_erro_invocacao(int argc, char** argv)
{printf("\n\taaaaa\n");
    int n_arg;
    /* Testa o número de argumentos da aplicação */
    n_arg = testa_numero_argumentos(argc);
    if(!n_arg) {return 0;} /*Se houver erro no nº de argumentos, retorna logo 0 (erro)*/
printf("\n\tbbbbbb\n");
    /*Testa o nome da aplicação*/
    if (strcmp(argv[0], "./ndn") != 0) {return 0;}
printf("\n\tccccccc\n");
    /*Testa se IP está no formato correcto*/ 
    if (!testa_formato_ip(argv[2])) {return 0;} /* Confere se está no formato XXX.XXX.XXX.XXX com o ultimo octeto entre 1 e 3 digitos*/
printf("\n\tdddddd\n");
    /*Testa se a porta TCP é válida*/
    if (!(atoi(argv[3])>0 && atoi(argv[3])<100000) ){return 0;}
printf("\n\teeeeeeeee\n");
    if(n_arg==4)
    {
        return 4;
    }
    else
    {
        if(n_arg==6)
        {    
         
            /*Testa se regIP está no formato correcto*/ 
  
            if (!testa_formato_ip(argv[4])) {return 0;} /* Confere se está no formato XXX.XXX.XXX.XXX com o ultimo octeto entre 1 e 3 digitos*/

            /*Testa se a porta regUDP é válida*/
            if (!(atoi(argv[5])>999 && atoi(argv[5])<100000) ) {return 0;}
           
        }
        else
        { 
            return 0;
        }

    }

    return n_arg; 
}