#include<sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/* ... */
#define max(A,B) ((A)>=(B)?(A):(B))

int select(int n, fd_set *readfds, fd_set *writefds, 
fd_set *exceptfds,struct timeval *timeout);
FD_CLR(int fd,fd_set *set);
FD_ISSET(int fd,fd_set *set);
FD_SET(int fd,fd_set *set);
FD_ZERO(fd_set *set);

int main(void)
{
int fd, newfd, afd=0;
fd_set rfds;
enum {idle,busy} state;
intmaxfd,counter;
/*...*/
/*fd=socket(…);bind(fd,…);listen(fd,…);*/
state=idle;
while(1){FD_ZERO(&rfds);
switch(state){
caseidle:FD_SET(fd,&rfds);maxfd=fd; break;
casebusy: FD_SET(fd,&rfds);FD_SET(afd,&rfds);maxfd=max(fd,afd); break;
}//switch(state)
counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
if(counter<=0)/*error*/exit(1);
for(;counter;--counter) 
switch(state){
caseidle:if(FD_ISSET(fd,&rfds)){FD_CLR(fd,&rfds);
addrlen=sizeof(addr);
if((newfd=accept(fd,&addr,&addrlen))==-1)/*error*/exit(1);
afd=newfd;state=busy;}
break;
casebusy: if(FD_ISSET(fd,&rfds)){FD_CLR(fd,&rfds);
addrlen=sizeof(addr);
if((newfd=accept(fd,&addr,&addrlen))==-1)/*error*/exit(1);
/* ... write “busy\n” in newfd */
close(newfd);}
else if(FD_ISSET(afd,&rfds)){FD_CLR(afd,&rfds);
if((n=read(afd,buffer,128))!=0)
{if(n==-1)/*error*/exit(1);
/* ... write buffer in afd */}
else{close(afd);state=idle;}//connection closed by peer
}
break;
}//switch(state)
}//while(1)
/*close(fd);exit(0);*/
}//main