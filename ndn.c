#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "ndn_headers.h"




int main(int argc, char** argv)
{
int i=0;
i = deteta_erro_invocacao(argc, argv);

INFO_NO no;
char regIP[tamanho_ip]="193.136.138.142", regUDP[tamanho_tcp]="59000";

switch (i)
{   
    case 6 : //regista regIP e regUDP
                strcpy(regIP, argv[4]);
                strcpy(regUDP, argv[5]);

    case 4 : // regista o id do nó
                strcpy(no.id.ip, argv[2]);
                strcpy(no.id.tcp, argv[3]);
            break;
    default :   printf("Erro na invocação da aplicação");
                exit(0);
            break; 
}

/*-_-_-_-_-_-_- AQUI ENTRA JÁ O SELECT -_-_-_-_-_-_-*/



return 0;

}