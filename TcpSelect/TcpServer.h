/*************************************************************************
    > File Name: TcpServer.h
    > Author: cjj
    > Created Time: 2013年10月27日 星期日 01时01分23秒
 ************************************************************************/
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "tcp_server_config.h"
#include <set>
#include <map>
#include <list>
#include <sys/select.h>
#include <pthread.h>

class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();
    //dealFunc是一个函数指针
    bool Initialize(unsigned int nPort, unsigned long dealFunc);
    bool SendData(const char * buf, size_t len, int sockfd);//向客户端发送数据,由于数据量小所以不用select,但是客户端不接收????
    bool RecvData(ServerData* sd);
    bool UnInitialize();

private:
    //pParam都是指向TcpServer实例的指针
    static void * AcceptThread(void * pParam); //select监是否有可读的sockfd
    static void * OperatorThread(void * pParam);//实际的业务处理
    static void * ManageThread(void * pParam);//监听 m_data集合是否有,有则创建一个OperatorThread
private:
    int m_server_socket;
    fd_set m_fdReads;
    pthread_mutex_t m_mutex;//锁是对下面的m_data使用的
    pCallBack m_operaFunc;
    //int m_client_socket[MAX_LISTEN];
    //std::set<int> m_client_socket; 使用下面的m_client代替
    std::list<ServerData> m_data;
    std::map<int,ServerData>  m_client;
    pthread_t m_pidAccept;
    pthread_t m_pidManage;
};

#endif
