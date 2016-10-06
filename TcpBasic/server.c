#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERV_PORT 25555
#define BACKLOG 20 //请求队列允许的最大请求数 
#define BUF_SIZE 256

int main(int argc,char *argv[])
{
	int ret;
	time_t tt;
	struct tm *ttm;
	char buf[BUF_SIZE];
	pid_t pid;
	int sockfd;
	int clientfd;
	struct sockaddr_in host_addr;
	struct sockaddr_in client_addr;
	int length = sizeof client_addr;
	
	//创建套接字
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == -1){
		printf("create socket failed.\n");
		return 0;
	}
	//绑定套接字
	bzero(&host_addr,sizeof(host_addr));
	host_addr.sin_family = AF_INET; //TCP/IP协议
	host_addr.sin_port = htons(SERV_PORT); 
	host_addr.sin_addr.s_addr = INADDR_ANY; //本机IP
	ret = bind(sockfd,(struct sockaddr *)&host_addr,sizeof host_addr);
	if (ret == -1){
		printf("call bind failed.\n");
		return 1;
	}
	//监听网络端口
	ret = listen(sockfd,BACKLOG);
	if (ret == -1){
		printf("call listen function failed.\n");
		return 1;
	}

	while (1){
		clientfd = accept(sockfd,(struct sockaddr *)&client_addr,&length);
		if (clientfd == -1){
			printf("call accept to receive connection failed\n");
			return 1;
		}
		pid = fork();
		if (pid == 0){
			while (1){
				bzero(buf,sizeof buf);
				tt = time(NULL);
				ttm = localtime(&tt);
				strcpy(buf,asctime(ttm));
				send(clientfd,buf,strlen(buf),0);
				printf("send info:%s\n",buf);
				sleep(2);
			}
			close(clientfd);
		}else if (pid>0){
			close(clientfd);
		}

	}
	return 0;
}
