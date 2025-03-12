#define _POSIX_C_SOURCE 200112L //remove os erros bruh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h> 
#include <ctype.h>
#include "ndn_headers.h"

#define ENTRY 0
#define SAFE 1
#define INTEREST 2
#define OBJECT 3
#define NOOBJECT 4


/**
 * @brief Envia uma mensagem formatada para um descritor de arquivo.
 * 
 * @param fd Descritor de arquivo de destino.
 * @param type Tipo da mensagem (ENTRY, SAFE, etc.).
 * @param name Nome ou IP do destinatário.
 * @param tcp Porta TCP do destinatário.
 */

 void mensagens(int fd, int type, char* name, char* tcp) {
    char* msg;
    char* msg_type;
    int length;

    switch(type) {
        case ENTRY:
            msg_type = "ENTRY";
            break;
        case SAFE:
            msg_type = "SAFE";
            break;
        case INTEREST:
            msg_type = "INTEREST";
            break;
        case OBJECT:
            msg_type = "OBJECT";
            break;
        case NOOBJECT:
            msg_type = "NOOBJECT";
            break;
        default:
            return;
    }

    if (type < INTEREST) {
        length = snprintf(NULL, 0, "%s %s %s\n", msg_type, name, tcp);
        msg = malloc(length + 1);
        sprintf(msg, "%s %s %s\n", msg_type, name, tcp);
    } else {
        length = snprintf(NULL, 0, "%s %s\n", msg_type, name);
        msg = malloc(length + 1);
        sprintf(msg, "%s %s\n", msg_type, name);
    }

    if (write(fd, msg, length) == -1) {
        perror("write");
    }

    printf("Sent to FD %d: %s\n", fd, msg);
    free(msg);
}

/**
 * @brief Atualiza o vizinho externo do nó.
 * 
 * @param fd Descritor de arquivo do novo vizinho externo.
 * @param ip Endereço IP do vizinho externo.
 * @param port Porta TCP do vizinho externo.
 * @param no Estrutura do nó a ser atualizada.
 */

 void atualiza_viz_externo(int fd, char* ip, char* port, INFO_NO* no) {
    no->id.fd = fd;
    strcpy(no->id.ip, ip);
    strcpy(no->id.tcp, port);
}


/**
 * @brief Atualiza as informações do nó salvo.
 * 
 * @param no Estrutura do nó a ser atualizada.
 * @param fd Estado do descritor de arquivo.
 * @param ip Endereço IP do nó salvo.
 * @param port Porta TCP do nó salvo.
 */

 void recebendo_safe(INFO_NO *no, ESTADO_FD fd, char* ip, char* port) {
    printf("[LOG] Recebendo safe: fd=%d, ip=%s, port=%s\n", fd, ip, port);
    
    no->no_salv.fd = fd;
    strcpy(no->no_salv.ip, ip);
    strcpy(no->no_salv.tcp, port);
    
    printf("[LOG] Estado salvo: fd=%d, ip=%s, port=%s\n", no->no_salv.fd, no->no_salv.ip, no->no_salv.tcp);
    printf("[LOG] ✅ Operação de safe concluída com sucesso.\n");
}


/**
 * @brief Processa a chegada de um nó ENTRY e atualiza as estruturas adequadamente.
 * 
 * @param no Estrutura do nó a ser atualizada.
 * @param fd Descritor de arquivo do nó que enviou ENTRY.
 * @param ip Endereço IP do nó que enviou ENTRY.
 * @param port Porta TCP do nó que enviou ENTRY.
 */

 void recebendo_entry(INFO_NO* no, int fd, char* ip, char* port) {
    printf("[LOG] Recebendo entry: fd=%d, ip=%s, port=%s\n", fd, ip, port);
    
    if (no->id.fd == SEM_CONEXAO) { // Primeira conexão, atualizar vizinho externo
        printf("[LOG] Primeira conexão detectada. Atualizando vizinho externo.\n");
        atualiza_viz_externo(fd, ip, port, no);
        recebendo_safe(no, PROPRIO_NO, no->id.ip, no->id.tcp);
        printf("[LOG] ✅ Vizinho externo atualizado com sucesso.\n");
    } else {
        INFO_NO novo_ext; // Criar estrutura na stack, sem malloc
        novo_ext.id.fd = fd;
        strcpy(novo_ext.id.ip, ip);
        strcpy(novo_ext.id.tcp, port);
        novo_ext.no_ext.fd = SEM_CONEXAO;
        novo_ext.no_salv.fd = SEM_CONEXAO;

        // Adicionar novo vizinho interno
        int i;
        for (i = 0; i < n_max_internos; i++) {
            if (no->no_int[i].fd == SEM_CONEXAO) { // Encontrar posição vazia
                no->no_int[i] = novo_ext.id; // Adicionar vizinho interno
                printf("[LOG] Vizinho interno adicionado na posição %d.\n", i);
                break;
            }
        }

        if (i == n_max_internos) {
            printf("[ERRO] ❌ Número máximo de vizinhos internos atingido!\n");
            return;
        }
    }
    printf("[LOG] 📩 Enviando mensagem SAFE para fd=%d, ip=%s, port=%s\n", fd, ip, port);
    mensagens(fd, SAFE, ip, port);
    printf("[LOG] ✅ Operação de entry concluída com sucesso.\n");
}
/*
 * testa_formato_porto - Verifica se a string fornecida representa uma porta TCP válida.
 *
 * Parâmetros:
 *   porto - String contendo o número da porta a ser validado.
 *
 * Retorno:
 *   0 - Se a porta for válida (número entre 0 e 65535).
 *   1 - Se a porta for inválida (não numérica, vazia ou fora do intervalo).
 */
int testa_formato_porto(char *porto) {
    if (porto == NULL || *porto == '\0') {
        return 1; // Inválido
    }

    for (int i = 0; porto[i] != '\0'; i++) {
        if (!isdigit(porto[i])) {
            return 1; // Inválido
        }
    }

    int numero = atoi(porto);
    if (numero < 0 || numero > 65535) {
        return 1; // Inválido
    }

    return 0; // Válido
}

/*
 * testa_formato_rede - Verifica se a string representa um identificador de rede válido (000-999).
 *
 * Parâmetros:
 *   net - String contendo o identificador da rede.
 *
 * Retorno:
 *   0 - Se for um identificador válido (formato "000" a "999").
 *   1 - Se for inválido (não numérico, tamanho incorreto ou fora do intervalo).
 */
int testa_formato_rede(char *net) {
    if (net == NULL || strlen(net) != 3) {
        return 1; // Inválido
    }

    for (int i = 0; i < 3; i++) {
        if (!isdigit(net[i])) {
            return 1; // Inválido
        }
    }

    int numero = atoi(net);
    if (numero < 0 || numero > 999) {
        return 1; // Inválido
    }

    return 0; // Válido
}

/** int testa_formato_ip
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

int join(char *rede_id, INFO_NO *no, char *regIP, char *regUDP) {
    // Isso aqui é UDP
    return 0;  
}

void show_topology(INFO_NO *no) {

    printf("========================================\n");
    
    if (no->id.fd != SEM_CONEXAO) {
        printf("🔗 Vizinho Externo: %s:%s\n\n", no->no_ext.ip, no->no_ext.tcp);
    } else {
        printf("[INFO] Atualmente sem vizinho externo.\n\n");
    }

    if (no->no_salv.fd != SEM_CONEXAO) {
        printf("🛡️  Vizinho de Salvaguarda: %s:%s\n\n", no->no_salv.ip, no->no_salv.tcp);
    } else {
        printf("[INFO] Atualmente sem vizinho de salvaguarda.\n\n");
    }
    
    printf("========================================\n\n");
}

int direct_join(char *rede_id, INFO_NO *no, char *connectIP, char *connectTCP, fd_set *master_set, int *max_fd) {
    
    printf("direct_join foi chamada com rede_id=%s, IP=%s, Port=%s\n", 
        rede_id, connectIP, connectTCP);

    int error = testa_formato_rede(rede_id);
    error = testa_formato_ip(connectIP);
    error = testa_formato_porto(connectTCP);
    
    if (error) {
        printf("Erro, formato do IP ou do porto não corresponde\n");
        return 1;
    }
    if (strcmp(connectIP, "0.0.0.0") == 0) {
        return 1; 
    }

    if (no->id.fd != SEM_CONEXAO) {
        printf("Already connected to a server. Aborting direct join.\n");
        return 1;
    }

    int errcode, n;
    struct addrinfo hints, *res;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errcode));
        return 1;
    }
    
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected to %s:%s on FD %d.\n", connectIP, connectTCP, fd);
    atualiza_viz_externo(fd, connectIP, connectTCP, no);  // Passando ponteiro corretamente
    printf("Updated external neighbour to %s:%s.\n", connectIP, connectTCP);

    FD_SET(fd, master_set);
    if (fd > *max_fd) *max_fd = fd;

    mensagens(fd, ENTRY, no->id.ip, no->id.tcp);
    return 0; 
}


/*
 * Função: parse_buffer
 * ---------------------
 * Divide o buffer lido e guarda cada palavra em um vetor de strings.
 *
 * buffer: String de entrada contendo palavras separadas por espaços.
 * words: Matriz de strings onde cada palavra será armazenada.
 */
void parse_buffer(char *buffer, int tamanho_buffer, char words[10][100]) {
    // Copia o vetor (porque usa strtok)
    char copia_buffer[tamanho_buffer]; 
    strncpy(copia_buffer, buffer, sizeof(copia_buffer) - 1);
    copia_buffer[sizeof(copia_buffer) - 1] = '\0';  
    
    // Usar strtok para dividir o buffer
    char *token = strtok(copia_buffer, " ");
    int index = 0;
    
    // Iterar pelos tokens e guardar no vetor words[][]
    while (token != NULL && index < 10) {
        // Caso a palavra termine com "\n", trocar por "\0"
        if (token[strlen(token) - 1] == '\n') {
            token[strlen(token) - 1] = '\0';
        }
        
        // Copiar para o vetor words[][]
        strncpy(words[index], token, 99);
        words[index][99] = '\0'; // Garantir terminação segura
        index++;
        token = strtok(NULL, " ");
    }
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