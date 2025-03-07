#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define tamanho_ip 16
#define tamanho_tcp 6
#define n_max_internos 50
#define n_max_obj 50
#define n_max_internos 50
#define tamanho_max_obj 100

/* -_-_-_-_-_-_-_ ESTRUTURAS DE DADOS PARA OS NÓS -_-_-_-_-_-_-_ */

/*------- Estrutura para o id de cada nó -------*/
typedef struct no
{
char ip[tamanho_ip];
char tcp[tamanho_tcp];
} ID_NO;

/*------- Estrutura para a informação do nó da aplicação -------*/
typedef struct info_no
{
ID_NO id;
ID_NO no_ext;
ID_NO no_salv;
ID_NO no_int[n_max_internos];
char cache[n_max_obj][tamanho_max_obj];
} INFO_NO;



int testa_numero_argumentos(int argc);
int testa_formato_ip(char* ip);
int deteta_erro_invocacao(int argc, char** argv);