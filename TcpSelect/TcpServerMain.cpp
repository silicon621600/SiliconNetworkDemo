#include "TcpServer.h"
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <string.h>

TcpServer tcpServer;
void deal(ServerData serverData)
{
    printf("开始处理\n");
    char res[MAX_SEND_DATA_LEN] = "0|";
    //if(NULL == data) return ; 传的结构体而非指针

    int len = strlen(serverData.data);
    if (serverData.remainDataLen!=0){
        //读入data有误; 错误信息已放在data里面
        tcpServer.SendData(serverData.data, len , serverData.sockfd);
        //别忘了关闭!!!
        close(serverData.sockfd);
        return ;
    }
    std::reverse(serverData.data,serverData.data+len);

    strcat(res,serverData.data);
    tcpServer.SendData(res,strlen(res),serverData.sockfd);
    printf("SendData:%s\n",res);
    //别忘了关闭!!!
    close(serverData.sockfd);
}



int main()
{
    if(false == tcpServer.Initialize(25555, (unsigned long)deal))
    {
        printf("Initialize failed\n");
        return -1;
    }
    printf("tcpserver:%ld\n", sizeof(tcpServer));
    while(1)
    {
        sleep(2);
    }
    return 0;
}
