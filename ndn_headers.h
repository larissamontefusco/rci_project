#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define tamanho_ip 16
#define tamanho_porto 6
#define n_max_internos 50
#define n_max_obj 50
#define tamanho_max_obj 100
#define n_max_nos 100



typedef enum {
    SEM_CONEXAO = -1,  // Sem conexão
    PROPRIO_NO = -2    // Representa o próprio nó
} ESTADO_FD;

/******************* Estrutura para identificação de um nó *******************/
/*
 * ID_NO - Estrutura que armazena as informações básicas de um nó na rede.
 * 
 * Campos:
 *   ip   - Endereço IP do nó.
 *   tcp  - Porta TCP utilizada pelo nó.
 *   fd   - Descritor de arquivo associado ao socket do nó.
 */
typedef struct no {
    char ip[tamanho_ip];   // Endereço IP do nó
    char tcp[tamanho_porto]; // Porta TCP do nó
    ESTADO_FD fd; // Descritor de arquivo do socket
} ID_NO;

/****************** Estrutura para armazenar informações de um nó ******************/
/*
 * INFO_NO - Estrutura que contém as informações completas de um nó na rede.
 * 
 * Campos:
 *   id        - Identificação do próprio nó (IP, porta, socket).
 *   no_ext    - Vizinhos externos (nó conectado diretamente fora da rede).
 *   no_salv   - Nó de salvaguarda, utilizado para backup de conexões.
 *   no_int    - Lista de vizinhos internos (conexões dentro da rede).
 *   cache     - Armazena objetos em cache dentro do nó.
 */
typedef struct info_no {
    ID_NO id;                         // Identificador do nó
    ID_NO no_ext;                      // Vizinho externo (conexão direta)
    ID_NO no_salv;                     // Nó de salvaguarda (backup)
    ID_NO no_int[n_max_internos];      // Lista de vizinhos internos
    char cache[n_max_obj][tamanho_max_obj]; // Cache de objetos armazenados
} INFO_NO;

/******************* Estrutura para representar uma rede *******************/
/*
 * REDE - Estrutura que representa uma rede composta por múltiplos nós.
 * 
 * Campos:
 *   id         - Identificação única da rede.
 *   nos_rede   - Lista de nós que pertencem à rede.
 *   total_nos  - Número total de nós atualmente na rede.
 */
typedef struct rede {
    char id[4];                        // Identificador da rede (ex: "020")
    INFO_NO nos_rede[n_max_nos];        // Lista de nós na rede
    int total_nos;                      // Contador de nós ativos
} REDE;


// Funções para teste de formato:
int testa_formato_ip(char* ip);
int testa_formato_porto(char *porto);
int testa_formato_rede(char *net);

// Funções da NDN
int direct_join(INFO_NO *no, char *connectIP, 
    char *connectPort, fd_set *fds, int *maxfd);
void show_topology(INFO_NO *no);
int create(char *name, INFO_NO *no);
void parse_buffer(char *buffer, int tamanho_buffer, char words[10][100]);
void recebendo_safe(INFO_NO *no, ESTADO_FD fd, char* ip, char* port);
void recebendo_entry(INFO_NO* no, int fd, char* ip, char* port);
void inicializar_no(INFO_NO *no);

bool testa_invocacao_programa(int argc, char** argv);
