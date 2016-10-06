#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERV_PORT 25555
#define BACKLOG 20 //请求队列允许的最大请求数 
#define BUF_SIZE 256

int main(int argc,char *argv[]){

	int ret;
	char buf[BUF_SIZE];
	int sockfd;
	struct sockaddr_in serv_addr;
	if (argc != 2){
		printf("command input error.\n");
		return 1;
	}
	//create socket
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == -1){
		printf("call socket function failed.\n");
		return 2;
	}

	//build conn
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);

	inet_aton(argv[1],(struct  sockaddr*)&serv_addr.sin_addr.s_addr,sizeof serv_addr);//set ip
	//bind socket
	ret = connect(sockfd,(struct sockaddr*)&serv_addr,sizeof serv_addr);
	if (ret == -1)
	{
		printf("call connect function failed\n");
		return 3;
	}
	while(1){
		bzero(buf,sizeof buf);
		recv(sockfd,buf,sizeof(buf),0);
		printf("recv info:%s",buf);
		sleep(1);
	}
	close(sockfd);
	return 0;
}
