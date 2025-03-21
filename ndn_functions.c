#define _POSIX_C_SOURCE 200112L
#include <stdbool.h>
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

bool testa_invocacao_programa(int argc, char** argv)
{
    if (argc != 4 && argc != 6) 
    {
        printf("[ERRO] ❌ Número de argumentos inválido!\n"); 
        return true;
    }

    int error = 0;
    error = testa_formato_ip(argv[2]);
    
    if (error) 
    {
        printf("[ERRO] ❌ Número de IP inválido!\n");
        return true;
    }

    error = testa_formato_porto(argv[3]);
    
    if (error) 
    {
        printf("[ERRO] ❌ Formato do porto TCP inválido!\n");
        return true;
    }

return false;
}


void inicializar_no(INFO_NO *no) {
    // Inicializa o nó principal como sem conexão
    no->id.fd = -1;

    // Inicializa o vizinho externo como sem conexão
    no->no_ext.fd = -1;

    // Inicializa o vizinho de salvaguarda como sem conexão
    no->no_salv.fd = -1;

    // Inicializa todos os vizinhos internos como sem conexão
    for (int i = 0; i < n_max_internos; i++) {
        no->no_int[i].fd = -1;
    }

    // Limpa chaches
    memset(no->cache, 0, sizeof(no->cache));
}


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

void atualiza_viz_interno(int fd, char* ip, char* port, INFO_NO* no) {
    printf("\n🔄 Atualizando vizinho interno...\n");

    // Procura um espaço disponível no array de vizinhos internos
    for (int i = 0; i < n_max_internos; i++) {
        if (no->no_int[i].fd == -1) {  // Encontra a primeira posição livre
            no->no_int[i].fd = fd;
            printf("✅ File descriptor atualizado: %d\n", fd);

            strcpy(no->no_int[i].ip, ip);
            printf("🌍 IP atualizado: %s\n", ip);

            strcpy(no->no_int[i].tcp, port);
            printf("🔗 Porta TCP atualizada: %s\n", port);

            printf("✅ Vizinho interno adicionado com sucesso! 🎉\n\n");
            return;
        }
    }

    // Caso o array esteja cheio
    printf("⚠️ Erro: Número máximo de vizinhos internos atingido!\n\n");
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
    printf("\n🔄 Atualizando vizinho externo...\n");

    no->no_ext.fd = fd;
    printf("✅ File descriptor atualizado: %d\n", fd);

    strcpy(no->no_ext.ip, ip);
    printf("🌍 IP atualizado: %s\n", ip);

    strcpy(no->no_ext.tcp, port);
    printf("🔗 Porta TCP atualizada: %s\n", port);

    printf("✅ Atualização concluída com sucesso! 🎉\n\n");
}


/**
 * @brief Atualiza as informações do nó salvo.
 * 
 * @param no Estrutura do nó a ser atualizada.
 * @param fd Estado do descritor de arquivo.
 * @param ip Endereço IP do nó salvo.
 * @param port Porta TCP do nó salvo.
 */

void recebendo_safe(INFO_NO *no, int fd, char* ip, char* port) {
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
    
    if (no->no_ext.fd == -1) { // Primeira conexão, atualizar vizinho externo
        printf("[LOG] Primeira conexão detectada. Atualizando vizinho externo.\n");
        atualiza_viz_externo(fd, ip, port, no);
        atualiza_viz_interno(fd, ip, port, no);
        recebendo_safe(no, -2, no->id.ip, no->id.tcp);
        printf("[LOG] ✅ Vizinho externo atualizado com sucesso.\n");
    } 
    else {
        // Adicionar novo vizinho interno
        atualiza_viz_interno(fd, ip, port, no);
        
        printf("[LOG] 📩 Enviando mensagem SAFE para fd=%d, ip=%s, port=%s\n", fd, ip, port);
        mensagens(no->no_ext.fd, SAFE, no->no_ext.ip, no->no_ext.tcp);
        printf("[LOG] ✅ Operação de entry concluída com sucesso.\n");
    }
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

/**
 * @brief Exibe a topologia da rede do nó atual.
 *
 * Esta função imprime na tela os vizinhos externos, internos e o vizinho de salvaguarda do nó.
 * Caso algum dos tipos de vizinhos não esteja presente, é exibida uma mensagem informativa.
 *
 * @param no Ponteiro para a estrutura INFO_NO que contém as informações sobre os vizinhos.
 */

void show_topology(INFO_NO *no) {
    printf("========================================\n");
    printf("🌐 Vizinhos Externos:\n");
    
    printf("🔗 Vizinho Externo: %s:%s\n\n", no->no_ext.ip, no->no_ext.tcp);

    if (no->no_salv.fd != -1) {
        printf("🛡️  Vizinho de Salvaguarda: %s:%s\n\n", no->no_salv.ip, no->no_salv.tcp);
    } else {
        printf("[INFO] Atualmente sem vizinho de salvaguarda.\n\n");
    }
    
    printf("🌐 Vizinhos Internos:\n");
    int tem_vizinhos = 0;
    for (int i = 0; i < n_max_internos; i++) {
        if (no->no_int[i].fd != -1) {
            printf("   - %s:%s\n", no->no_int[i].ip, no->no_int[i].tcp);
            tem_vizinhos = 1;
        }
    }
    if (!tem_vizinhos) {
        printf("   [INFO] Nenhum vizinho interno encontrado.\n");
    }
    
    printf("========================================\n\n");
}

/**
 * @brief Entra em uma rede especificada enviando uma solicitação para o servidor.
 *
 * @param net    Identificador da rede (deve estar no formato 000-999).
 * @param no     Estrutura contendo informações do nó atual (IP e porta TCP).
 * @param regIP  Endereço IP do servidor de registro.
 * @param regUDP Porta UDP do servidor de registro.
 * @return int   Retorna 0 em caso de sucesso, ou -1 em caso de erro.
 */

 int join(char *net, INFO_NO *no, char *regIP, char *regUDP, fd_set *master_set, int *max_fd) {
    printf("[INFO] Iniciando processo de entrada na rede %s...\n", net);

    // Verifica se o formato da rede está correto
    int error = testa_formato_rede(net);
    if (error) {
        printf("[ERRO] Formato da rede %s não está correto. (Deve estar entre 000-999)\n", net);
        return -1;
    }
    
    struct addrinfo hints, *res;
    ID_NO no_a_ligar;
    int fd, errcode;
    ssize_t n;
    char msg[128]="NODES ", msg_2[128]="REG ", buffer[128 + 1];
    char str1[9] ="", str2[3] ="", str3[tamanho_ip]="", str4[tamanho_porto]="";
    
    // Concatena o nome da rede na mensagem de solicitação de nós
    strcpy(msg, (strcat(msg, net)));
    
    // Criar socket UDP
    printf("[INFO] Criando socket UDP...\n");
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("[ERRO] Falha ao criar socket");
        return -1;
    }

    // Configuração da estrutura hints para obtenção de endereço do servidor
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // Obter endereço do servidor
    printf("[INFO] Obtendo endereço do servidor %s:%s...\n", regIP, regUDP);
    errcode = getaddrinfo(regIP, regUDP, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "[ERRO] getaddrinfo: %s\n", gai_strerror(errcode));
        close(fd);
        return -1;
    }

    printf("[INFO] Mensagem a ser enviada: %s\n", msg);

    // Enviar mensagem de solicitação de nós ao servidor
    printf("[INFO] Enviando mensagem ao servidor...\n");
    n = sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("[ERRO] Falha ao enviar mensagem");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    printf("[SUCESSO] Mensagem enviada com sucesso.\n");

    // Receber resposta do servidor
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    printf("[INFO] Aguardando resposta do servidor...\n");
    n = recvfrom(fd, buffer, 128, 0, &addr, &addrlen);
    if (n == -1) {
        perror("[ERRO] Falha ao receber resposta do servidor");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    buffer[n] = '\0';
    printf("[SUCESSO] Resposta do servidor recebida: %s\n", buffer);
    
    // Lê o primeiro nó da rede, se houver, e tenta se conectar a ele
    if((sscanf(buffer, "%s %s %s %s", str1, str2, str3, str4)) == 4)
    {
        strcpy(no_a_ligar.ip, str3);
        strcpy(no_a_ligar.tcp, str4); 

        // Liga-se ao nó escolhido
        if(direct_join(no, no_a_ligar.ip, no_a_ligar.tcp, master_set, max_fd))
        {
            printf("Correu mal o direct join!");
            freeaddrinfo(res);
            close(fd);
            return -1;
        }    
    }
    
    // Compõe a mensagem de registro ao servidor
    strcpy(msg_2, (strcat(msg_2, net)));
    strcpy(msg_2, (strcat(msg_2, " ")));
    strcpy(msg_2, (strcat(msg_2, no->id.ip)));
    strcpy(msg_2, (strcat(msg_2, " ")));
    strcpy(msg_2, (strcat(msg_2, no->id.tcp)));
    printf("[INFO] Mensagem 2 a ser enviada: %s\n\n", msg_2);

    // Enviar mensagem de registro ao servidor
    n = sendto(fd, msg_2, strlen(msg_2), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("[ERRO] Falha ao enviar mensagem");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    printf("[SUCESSO] Mensagem enviada com sucesso.\n");

    // Aguardar resposta de confirmação do servidor
    printf("[INFO] Aguardando resposta do servidor...\n");
    n = recvfrom(fd, buffer, 128, 0, &addr, &addrlen);
    if (n == -1) {
        perror("[ERRO] Falha ao receber resposta do servidor");
        freeaddrinfo(res);
        close(fd);
        return -1;
    }
    buffer[n] = '\0';
    printf("[SUCESSO] Resposta do servidor recebida: %s\n", buffer);
    
    // Liberar recursos
    freeaddrinfo(res);
    close(fd);
    printf("[INFO] Processo de entrada na rede concluído com sucesso.\n");
    return 0;
}

int direct_join(INFO_NO *no, char *connectIP, char *connectTCP, fd_set *master_set, int *max_fd) {
    
    printf("direct_join foi chamada com IP=%s, Porta=%s\n", connectIP, connectTCP);

    int error = testa_formato_ip(connectIP);
    
    if (error) {
        printf("Erro: formato do IP está incorreto\n");
        return -1;
    }
    if (!strcmp(connectIP, no->id.ip) && !strcmp(connectTCP, no->id.tcp)){
        printf("Erro: Se quiser utilizar o dj com o próprio nó, deves digitar dj 0.0.0.0\n");
        return -1;
    }
    
    if (strcmp(connectIP, "0.0.0.0") == 0) {
        no->no_ext.fd = no->id.fd;
        strcpy(no->no_ext.ip, no->id.ip);
        strcpy(no->no_ext.tcp, no->id.tcp);
        printf("O nó agora é seu próprio vizinho externo.\n");
        return 0;  
    }

    error = testa_formato_porto(connectTCP);
    if (error) {
        printf("Erro: formato da porta TCP está incorreto\n");
        return -1;
    }

    if (no->id.fd != -1) {
        printf("Já está conectado a um servidor. Abortando conexão direta.\n");
        return -1;
    }

    int errcode, n;
    struct addrinfo hints, *res;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Erro ao criar socket");
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    errcode = getaddrinfo(connectIP, connectTCP, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "Erro no getaddrinfo: %s\n", gai_strerror(errcode));
        return -1;
    }
    
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("Erro ao conectar");
        return -1;
    }

    printf("Conectado a %s:%s no descritor de arquivo %d.\n", connectIP, connectTCP, fd);
    
    atualiza_viz_externo(fd, connectIP, connectTCP, no);
    atualiza_viz_interno(fd, connectIP, connectTCP, no);

    printf("Vizinho externo atualizado para %s:%s.\n", connectIP, connectTCP);
    printf("Meu nó = %s %s", no->id.tcp, no->id.ip);

    FD_SET(fd, master_set);
    if (fd > *max_fd) *max_fd = fd;

    mensagens(fd, ENTRY, no->id.ip, no->id.tcp);
    recebendo_safe(no, -2, no->id.ip, no->id.tcp);

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

// Funções para gerenciar o cache
/**
 * @brief Armazena um objeto no cache.
 * 
 * A função verifica se há espaço disponível no cache e armazena o objeto, caso contrário, retorna um erro.
 * 
 * @param name Nome do objeto a ser armazenado.
 * @param no Ponteiro para a estrutura INFO_NO que contém o cache.
 * @return 0 se o objeto for armazenado com sucesso, 1 caso contrário.
 */

int create(char *name, INFO_NO *no) {
    if (strlen(name) >= tamanho_max_obj) {
        printf("Erro: Nome muito grande.\n");
        return 1;
    }

    // Encontrar um espaço livre no cache
    for (int i = 0; i < n_max_obj; i++) {
        if (no->cache[i][0] == '\0') { // Se a posição estiver vazia
            strcpy(no->cache[i], name);
            printf("[LOG] ✅ Objeto '%s' armazenado na posição %d do cache com sucesso.\n", name, i);
            return 0;
        }
    }

    printf("Erro: Cache cheio.\n");
    return 1; 
}

/**
 * @brief Remove um objeto do cache.
 * 
 * A função busca o objeto no cache e o remove caso ele seja encontrado. Se o objeto não existir, retorna um erro.
 * 
 * @param name Nome do objeto a ser removido.
 * @param no Ponteiro para a estrutura INFO_NO que contém o cache.
 * @return 0 se o objeto for removido com sucesso, 1 caso contrário.
 */

int delete(char *name, INFO_NO *no) {
    for (int i = 0; i < n_max_obj; i++) {
        if (strcmp(no->cache[i], name) == 0) { // Encontrou o objeto
            no->cache[i][0] = '\0'; // Marca como vazio
            printf("[LOG] ❌ Objeto '%s' removido da posição %d do cache com sucesso.\n", name, i);
            return 0;
        }
    }
    
    printf("Erro: Objeto '%s' não encontrado no cache.\n", name);
    return 1; 
}

/**
 * @brief Exibe todos os objetos armazenados no cache.
 * 
 * A função percorre o cache e exibe todos os nomes armazenados.
 * 
 * @param no Ponteiro para a estrutura INFO_NO que contém o cache.
 */

void show_names(INFO_NO *no) {
    printf("[LOG] 📜 Objetos armazenados no cache:\n");
    int found = 0;
    for (int i = 0; i < n_max_obj; i++) {
        if (no->cache[i][0] != '\0') { // Se a posição não estiver vazia
            printf("- %s\n", no->cache[i]);
            found = 1;
        }
    }
    if (!found) {
        printf("(Nenhum objeto armazenado)\n");
    }
}
