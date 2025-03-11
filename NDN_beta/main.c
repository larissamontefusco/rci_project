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
#include "fun.h"
#define BACKLOG 5      // Maximum number of pending connections
#define BUFFER_SIZE 128

#define ENTRY_msg 0
#define SAFE_msg 1
#define INTEREST_msg 2
#define OBJECT_msg 3
#define NOOBJECT_msg 5

typedef struct s_node{
    int fd;
    char ip[20];
    char port[10];
    struct s_node* next;
} s_node;

// Inicializar os nodes
s_node n_local = {-1, "", "", NULL};  
s_node vz_ext  = {-1, "", "", NULL};
s_node vz_salv = {-1, "", "", NULL};
s_node vz_int  = {-1, "", "", NULL};


/**
 * No caso do vz_salv, o fd is an indicator of
 * -1 : no connection
 * -2 : itself
 */

fd_set* master_set; // ponteiro para o master_fds
int max_fd;


/* envia uma mensagem, name:= ip OU objeto */
void f_msg(int fd, int type, char* name, char* tcp){
    char* msg;
    char* msg_type;
    int length = 0;

    switch(type){
        case ENTRY_msg: 
            msg_type = "ENTRY";
            break;
        case SAFE_msg:
            msg_type = "SAFE";
            break;
        case INTEREST_msg:
            msg_type = "INTEREST";
            break;
        case OBJECT_msg:
            msg_type = "OBJECT";
            break;
        case NOOBJECT_msg:
            msg_type = "NOOBJECT";
            break;
        default:
            exit(1);
    }

    if (type < INTEREST_msg){
        length = snprintf(NULL, 0, "%s %s %s\n", msg_type, name, tcp); 
        msg = (char*)malloc(length + 1);  // +1 for the null terminator
        sprintf(msg, "%s %s %s\n", msg_type, name, tcp);
    } else {
        length = snprintf(NULL, 0, "%s %s\n", msg_type, name); 
        msg = (char*)malloc(length + 1);  // +1 for the null terminator
        sprintf(msg, "%s %s\n", msg_type, name);
    }

    int ret;
    ret = write(fd, msg, length);

    if (ret == -1){
        perror("write");
    }

    printf(GREEN "[SENT TO FD %d] " RESET "%s\n", fd, msg);
    //printf("SENT %d\n", ret);

    free(msg);
}


void initial_printf(){
    printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "AVAILABLE COMMANDS:\n" RESET);
    printf("- direct join (dj) IP PORT\n");
    printf("- show topology (st)\n");
    printf("- exit (x)\n");
}


/* show topology */
void f_show_topology() {
    s_node *head = vz_int.next;

    if (vz_ext.fd != -1){
        printf(WHITE_BOLD "Vizinho Externo:" RESET "\t%s:%s\n\n", vz_ext.ip, vz_ext.port);
    } else printf(YELLOW "[INFO] " RESET  "Atualmente sem vizinho externo.\n");

    if (vz_salv.fd != -1){
        printf(WHITE_BOLD "Vizinho de Salvaguarda:" RESET "\t%s:%s\n\n", vz_salv.ip, vz_salv.port);
    } else printf(YELLOW "[INFO] " RESET "Atualmente sem vizinho de salvaguarda.\n");

    if (head == NULL){
        printf(YELLOW "[INFO] " RESET "Atualmente sem vizinhos internos.\n");
    } else {  
        printf(WHITE_BOLD "Vizinhos Internos:\n" RESET);
        while (head != NULL) {
            printf("\t\t\t%s:%s\n", head->ip, head->port);
            head = head->next;
        } 
        printf("\n");
    }
    printf("\n");
}

/* atualiza a estrutura do vizinho externo */
void f_update_vz_ext(int fd, char* ip, char* port){
    vz_ext.fd = fd;
    strcpy(vz_ext.ip,ip); 
    strcpy(vz_ext.port,port); 
}

/* direct join */
void f_direct_join(char* ip, char* port){

    if (strcmp(ip, "0.0.0.0") == 0) {
        // FALTA AQUI ALGUMA CENA
        return;
    }

    if(vz_ext.fd != -1){
        printf(YELLOW "[INFO] " RESET "Already connected to a server.\n");
        printf(YELLOW "[INFO] " RESET "Aborting direct join.\n\n");
        return;
    }

    int errcode;
    int n;
    struct addrinfo hints, *res;

    // Create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    // Get address info
    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errcode));
        return;
    }
    
    // Connect to the server
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("connect");
        exit(1);
    }

    printf(YELLOW "[INFO] " RESET "Connected to %s:%s on FD %d.\n", ip, port, fd);
    f_update_vz_ext(fd, ip, port); // atualizar o seu vizinho externo
    printf(YELLOW "[INFO] " RESET "Updated external neighbour to %s:%s.\n", ip, port);

    FD_SET(fd,master_set); // adicionar este FD a lista de master_fds
    if (fd > max_fd){ 
        max_fd = fd; // caso for o maior fd, atualizar o valor de maior fd
    }

    f_msg(fd, ENTRY_msg, n_local.ip, n_local.port); // enviar mensagem ENTRY
}

/* divide o buffer lido, e guarda cada palavra num vetor words[][] */
void parse_buffer(char *buffer, char words[6][100]) {

    // copia o vetor (porque usa strtok)
    char buffer_copy[BUFFER_SIZE]; 
    strncpy(buffer_copy, buffer, sizeof(buffer_copy) - 1);
    buffer_copy[sizeof(buffer_copy) - 1] = '\0';  
    
    // usar strtok para dividir o buffer
    char *token = strtok(buffer_copy, " ");
    int index = 0;
    
    // iterar pelos tokens, e guardar no vetor words[][]
    while (token != NULL && index < 6) {
        
        if (token[strlen(token) - 1] == '\n') {
            token[strlen(token) - 1] = '\0'; // caso a palavra termine com "\n", trocar por "\0"
        }
        
        strncpy(words[index], token, 99);  // copiar para o vetor words[][]
        index++;
        token = strtok(NULL, " ");
    }
}

/* lida com a resposta a mensagem SAFE */
void f_safe(int fd, char* ip, char* port) {
    vz_salv.fd = fd;
    strcpy(vz_salv.ip,ip);
    strcpy(vz_salv.port,port);
}

/* lida com a resposta a mensagem ENTRY */
void f_entry(int fd, char* ip, char* port) {

    if (vz_ext.fd == -1){ // caso seja a primeira conexao, atualizar vizinho externo
        f_update_vz_ext(fd,ip,port);
        f_safe(-2,n_local.ip,n_local.port);
    } else {
        s_node* new_viz = (s_node*)malloc(sizeof(s_node)); // allocar memoria
        if (!new_viz) {
            perror("malloc failed");
            return;
        }

        new_viz->fd = fd; // Proper assignment
        strcpy(new_viz->ip, ip);
        strcpy(new_viz->port, port);
        new_viz->next = NULL;

        if (vz_int.next == NULL) {
            vz_int.next = new_viz; // primeiro vizinho interno
        } else {
            s_node* current = vz_int.next;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_viz; // restantes vizinhos internos
        }
    }

    f_msg(fd,SAFE_msg,vz_ext.ip,vz_ext.port);

}

/* lida com qualquer leitura do main */
int f_process(int fd, char* buffer){
    char words[6][100];
    parse_buffer(buffer, words);

    if (fd == STDIN_FILENO){
        printf("\n");
        if(strcmp("direct", words[0]) == 0 && strcmp("join",words[1]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "DIRECT JOIN\n\n" RESET);
            f_direct_join(words[2],words[3]);
            return 0;
        }
        if(strcmp("dj", words[0]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "DIRECT JOIN\n\n" RESET);
            f_direct_join(words[1],words[2]);
            return 0;
        }
        if(strcmp("show", words[0]) == 0 && strcmp("topology",words[1]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "SHOW TOPOLOGY\n\n" RESET);
            f_show_topology();
            return 0;
        }
        if(strcmp("st", words[0]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "SHOW TOPOLOGY\n\n" RESET);
            f_show_topology();
            return 0;
        }
        if(strcmp("x",words[0]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "EXIT\n\n" RESET);
            return 1;
        }
        if(strcmp("exit",words[0]) == 0){
            printf(MAGENTA_BOLD "[APP] " WHITE_BOLD "EXIT\n\n" RESET);
            return 1;
        }
    } else {
        if(strcmp("ENTRY", words[0]) == 0){
            f_entry(fd,words[1],words[2]);
            return 0;
        }
        if(strcmp("SAFE", words[0]) == 0){
            f_safe(fd,words[1],words[2]);
            printf("\n");
            return 0;
        }

    }

    printf(YELLOW "[INFO] " RESET "This command isn't available: %s\n", buffer);
    initial_printf();
    printf("\n");
    return 0;
}

/* liberta a memoria alocada dos vizinhos internos */
void f_free_vz() {
    s_node* current = vz_int.next; // comeca com o primeiro vizinho alocado (segundo vizinho interno)
    s_node* temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }

    vz_int.next = NULL; 
}

/* remove um vizinho apenas */
void f_remove_vz(int fd) {

    s_node* current = vz_int.next;
    s_node* prev = NULL;

    while (current != NULL) {
        if (current->fd == fd) {  
            if (prev == NULL) {  
                vz_int.next = current->next; 
            } else {  
                prev->next = current->next; 
            }
            free(current); 
            //printf("Node with fd %d removed.\n", fd);
            return;
        }
        prev = current;
        current = current->next;
    }
    
}

void f_handle_disconnection(int fd){

    // se o que se desconectou foi o externo
    if (fd == vz_ext.fd){
        printf(YELLOW "[INFO] " RESET "O vizinho externo no FD %d disconectou-se.\n", fd);
        if(vz_salv.fd == -2){ // caso tiver como si mesmo o salvaguarda
            printf(YELLOW "[INFO] " RESET "O utilizador tem-se a si mesmo como nó de salvaguarda.\n\n");
            f_update_vz_ext(-1,"0","0");
            f_safe(-1,"0","0");
            return;
        } else {
            printf(YELLOW "[INFO] " RESET "Conectando com o seu nó de salvaguarda.\n");
            f_update_vz_ext(-1,vz_salv.ip,vz_salv.port);
            f_direct_join(vz_salv.ip,vz_salv.port);
            f_safe(-1,"0","0");
            return;
        } 
    } else {
        f_remove_vz(fd); // se foi um interno
        printf(YELLOW "[INFO] " RESET "O vizinho interno no FD %d disconectou-se.\n\n", fd);
        return;
    }
}

// para correr com o comando "./ndn CACHE IP TCP regIP regTCP"
int main(int argc, char* argv[]){

    // lidar com os argumentos
    if (argc != 4 && argc != 6){
        printf("Correr com ./ndn CACHE IP TCP (regIP regTCP)\n");
        return 1;
    }
    
    strcpy(n_local.ip,argv[2]);
    strcpy(n_local.port,argv[3]);

    int listener_fd, new_fd, counter;
    struct addrinfo hints, *res;
    struct sockaddr addr;
    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    fd_set master_fds, read_fds; // File descriptor sets

    if ((listener_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit(1);
    
    // Setting up the socket
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_INET;        
    hints.ai_socktype = SOCK_STREAM;  // TCP socket
    hints.ai_flags = AI_PASSIVE;      

    // Get address info for binding the socket
    if (getaddrinfo(NULL, n_local.port, &hints, &res) != 0) {
        perror("getaddrinfo");
        printf(RED "[ERROR] " RESET "PORT ERROR | User's TCP port argument: \"%s\"\n", n_local.port);
        exit(1);
    }

    // Bind socket to the specified port
    if (bind(listener_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        exit(1);
    }
    // Create a socket
    if ((listener_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("socket");
        exit(1);
    }

    
    
    // No longer needed, so we free the structure
    freeaddrinfo(res); 

    // Start listening for incoming connections
    if (listen(listener_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    // Set up file descriptors
    FD_ZERO(&master_fds); 	
    FD_SET(listener_fd, &master_fds); 
    FD_SET(STDIN_FILENO, &master_fds);  // Add stdin to the fd set
    max_fd = (listener_fd > STDIN_FILENO) ? listener_fd : STDIN_FILENO; // Determine the max FD value
    int result_exit;
    master_set = &master_fds;

    printf(YELLOW "[INFO] " RESET "Server is listening on port %s...\n\n", n_local.port);
    initial_printf();
    printf("\n");

    while (1) {

        if (result_exit == 1){
            break;
        }

        read_fds = master_fds; // Set the master set at each iteration
        
        counter = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (counter == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= max_fd; i++) {

            memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer before reading new data 

            if (result_exit == 1){
                break;
            }
            // check if "i" is set as a file descriptor FD_ISSET
            if (FD_ISSET(i, &read_fds)) { 
                // CASE 1: New connection request on the listener socket
                if (i == listener_fd) {              
                    addrlen = sizeof addr;
                    new_fd = accept(listener_fd, &addr, &addrlen);
                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(new_fd, &master_fds);
                        if (new_fd > max_fd) max_fd = new_fd;
                        printf(YELLOW "[INFO] " RESET "New connection established: FD %d\n\n", new_fd);
                    }
                }

                // CASE 2: Data received from an existing client
                else if (i == STDIN_FILENO) {
                    // Read a line of text from the terminal
                    fgets(buffer, BUFFER_SIZE, stdin);
                    system("clear");
                    printf(WHITE_BOLD "[USER] " RESET "%s", buffer);
                    printf("\n────────────────────────────────────────────────────\n");
                    result_exit = f_process(i, buffer);

                } else {
                    int n = read(i, buffer, BUFFER_SIZE);

                    if (n <= 0) {                   // Connection closed or error
                        if (n == 0) {
                            printf(YELLOW "[INFO] " RESET "Client on FD %d disconnected.\n", i);
                            f_handle_disconnection(i);
                        } else {
                            perror("read");
                        }
                        close(i);
                        FD_CLR(i, &master_fds);     // Remove closed connection from master set
                    } else {
                        printf(CYAN "[READ FROM FD %d] " RESET "%s", i, buffer);
                        result_exit = f_process(i, buffer);
                    }
                }
                printf("────────────────────────────────────────────────────\n\n");
            }
        }
    }
    

    f_free_vz();
    close(listener_fd);

    printf(YELLOW "[INFO] " RESET "Successful exit with return code 0.\n");

    return 0;
}

