#ifndef TCP_SERVER_CONFIG_H
#define TCP_SERVER_CONFIG_H
#include <stddef.h>
#include <time.h>
const int MAX_LISTEN = 10000;//要支持10000的并发,???
const int SERVER_LISTEN_PORT = 25555;
const int MAX_RECV_DATA_LEN =4096;
const int MAX_SEND_DATA_LEN =4096;
const int BUF_SIZE = 256;
const int MAXEVENTS = 64;
const int MAX_CONN_TIME = 10;//每个连接维持的最长时间
typedef struct
{
    char data[MAX_RECV_DATA_LEN]; //不用unsigned char,默认处理字符流
    int remainDataLen; // not use size_t 因为我需要负数 还剩多少数据未读
    int sockfd;
    time_t startTime;//连接的时间
}ServerData, *pServerData;

//定义函数指针,使用回调函数机制,该函数对应服务端的计算函数
//typedef void (*pCallBack)(const char * szBuf, size_t nLen, int socket); 原先代码使用const char* 里面再强转,有什么好处吗???
typedef void (*pCallBack)(ServerData data);

#endif
