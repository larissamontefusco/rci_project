/*Só funciona se tiver o servidor enviando aberto em outro pc*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <string.h>
#include <unistd.h>
int main(void)
{
    int fd;
    struct sockaddr addr;
    socklen_t addrlen;
    ssize_t n;
    char buffer[128+1];
    /*...*///see previous task code
    addrlen=sizeof(addr);
    n=recvfrom(fd,buffer,128,0,&addr,&addrlen);
    if(n==-1)/*error*/exit(1);
    buffer[n] = '\0';
    printf("echo: %s\n", buffer);
    close(fd);
    exit(0);
}