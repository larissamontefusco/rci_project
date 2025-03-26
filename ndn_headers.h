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
#define n_max_interfaces (n_max_internos + 1) // É o número de internos + externo
#define n_max_nos 100
#define n_max_interests 100
#define tamanho_max_obj 100
extern int n_max_obj;

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
    int fd; // Descritor de arquivo do socket
} ID_NO;


/****************** Estrutura para armazenar interesses pendentes ******************/
/*
    INTEREST - Estrutura que contém as informações sobre os interesses
    Campos:
    name - Identificação dos objetos de interesse
    interfaces - Armazena as interfaces que tem comunicação com o nó
*/
typedef struct {
    char name[tamanho_max_obj];  
    int interfaces[n_max_interfaces]; 
} INTEREST;

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

 typedef struct info_net {
    char net_id[4];                         // Identificador da rede
    char regIP[20];
    char regUDP[7];
} INFO_NET;

typedef struct info_no {
    ID_NO id;                         // Identificador do nó
    ID_NO no_ext;                      // Vizinho externo (conexão direta)
    ID_NO no_salv;                     // Nó de salvaguarda (backup)
    ID_NO no_int[n_max_internos];      // Lista de vizinhos internos
    char **cache; // Cache de objetos armazenados
    int num_objetos; // Número de objetos
    INTEREST interests[n_max_interests]; // Tabela de interesses pendentes
    int num_interesses;                // Número atual de interesses ativos // Número de interesses
    INFO_NET net;
} INFO_NO;



// Funções para teste de formato:
int testa_formato_ip(char* ip);
int testa_formato_porto(char *porto);
int testa_formato_rede(char *net);

// Funções da NDN
int join(char *net, INFO_NO *no, char *regIP, char *regUDP, fd_set *master_set, int *max_fd);
int direct_join(INFO_NO *no, char *connectIP, char *connectPort, fd_set *fds, int *maxfd);
void show_topology(INFO_NO *no);
int create(char *name, INFO_NO *no);
int delete(char *name, INFO_NO *no);
void parse_buffer(char *buffer, int tamanho_buffer, char words[10][100]);
void recebendo_safe(INFO_NO *no, int fd, char* ip, char* port);
void recebendo_entry(INFO_NO* no, int fd, char* ip, char* port);
void recebendo_interesse(INFO_NO *no, char *objeto, int origem_interface);
void recebendo_objeto(INFO_NO *no, char *objeto, int origem_interface);
void recebendo_noobjeto(INFO_NO *no, char *objeto, int origem_interface);
void inicializar_no(INFO_NO *no);
bool testa_invocacao_programa(int argc, char** argv);
void show_names(INFO_NO *no);
void show_interest_table(INFO_NO *no);
int leave(INFO_NO *no);
int retrieve(char *name, INFO_NO *no);
