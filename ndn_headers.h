#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define tamanho_ip 16
#define tamanho_porto 6
#define n_max_internos 50
#define n_max_obj 50
#define tamanho_max_obj 100
#define n_max_nos 100

/******************* ESTRUTURAS DE DADOS PARA OS NÓS *******************/

/******************* Estrutura para o id de cada nó *******************/
typedef struct no
{
    char ip[tamanho_ip];   // Endereço IP do nó
    char tcp[tamanho_porto]; // Porta TCP do nó
} ID_NO;

/****************** Estrutura para a informação do nó da aplicação ******************/
typedef struct info_no
{
    ID_NO id;                         // Identificador do nó
    ID_NO no_ext;                      // Identificador do vizinho externo
    ID_NO no_salv;                     // Identificador do nó de salvaguarda
    ID_NO no_int[n_max_internos];      // Lista de vizinhos internos
    char cache[n_max_obj][tamanho_max_obj]; // Cache de objetos armazenados
} INFO_NO;


typedef struct rede 
{
    int id;
    INFO_NO nos_rede[n_max_nos];
    int total_nos;
} REDE;
/****************** FUNÇÕES AUXILIARES ******************/

int testa_formato_ip(char* ip);
int join(REDE *rede, INFO_NO *no);
int direct_join();
int create(char *name, INFO_NO *no);

