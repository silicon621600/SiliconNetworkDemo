#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>//atoi()
#include <arpa/inet.h>//inet_ntoa()

#define MAX_MSG_SIZE 1024
#define SERVER_PORT 8888
char msg[MAX_MSG_SIZE];
int client(int argc,char **argv)
{
    int sockfd, port,n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    if (argc!=3)
    {
        printf("command params error.\n");
        return 1;
    }
    if ((port=atoi(argv[2]))<0)
    {
        printf("input port error.\n");
        return 1;
    }
    if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        perror("socket error.");
        return 1;
    }
    addrlen = sizeof (struct sockaddr_in);
    bzero(&addr,addrlen);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(argv[1],&addr.sin_addr)<0)
    {
        printf("IP error.");
        return 1;
    }
    while (1)
    {
        bzero(msg,MAX_MSG_SIZE);
        fgets(msg,MAX_MSG_SIZE,stdin);
        sendto(sockfd,msg,strlen(msg),0,(struct sockaddr*)(&addr),addrlen);
        printf("send msg:%s",msg);
        n=recvfrom(sockfd,msg,sizeof(msg),0,(struct sockaddr*)(&addr),&addrlen);
        printf("recv msg:%s",msg);
    }
    close(sockfd);
    return 0;
}
int server()
{
    int sockfd, port,n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    port = SERVER_PORT;
    if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        perror("socket error.");
        return 1;
    }
    addrlen = sizeof (struct sockaddr_in);
    bzero(&addr,addrlen);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd,(struct sockaddr*)(&addr),addrlen)<0)
    {
        printf("bind error.");
        return 1;
    }

    while (1)
    {
        bzero(msg,MAX_MSG_SIZE);
        n=recvfrom(sockfd,msg,sizeof(msg),0,(struct sockaddr*)(&addr),&addrlen);
        printf("recv msg:%s",msg);
        fgets(msg,MAX_MSG_SIZE,stdin);
        sendto(sockfd,msg,strlen(msg),0,(struct sockaddr*)(&addr),addrlen);
        printf("send msg:%s",msg);
   }
    close(sockfd);
    return 0;
}
int main(int argc,char **argv)
{
    int ret;
    if (argc>1)
    {
        ret=client(argc,argv);
    }else
    {
        ret=server();
    }
    return ret;
}

