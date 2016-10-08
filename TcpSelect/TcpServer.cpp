
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "TcpServer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

TcpServer::TcpServer()
{
    pthread_mutex_init(&m_mutex, NULL);
    FD_ZERO(&m_fdReads);
    m_client.clear();
    m_data.clear();
    m_operaFunc = 0;
    m_pidAccept = 0;
    m_pidManage = 0;
}

TcpServer::~TcpServer()
{
    FD_ZERO(&m_fdReads);
    m_client.clear();
    m_data.clear();
    m_operaFunc = NULL;
    pthread_mutex_destroy(&m_mutex);
}

bool TcpServer::Initialize(unsigned int nPort, unsigned long dealFunc)
{
    if(0 != dealFunc)
    {
        //设置回调函数
        m_operaFunc = (pCallBack)dealFunc;
    }
    //先反初始化
    UnInitialize();
    //创建socket
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == m_server_socket)
    {
        printf("socket error:%m\n"); //%m 对应errno
        return false;
    }
    //绑定IP和端口
    sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(nPort);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res = bind(m_server_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == res)
    {
        printf("bind error:%m\n");
        return false;
    }
    //监听
    res = listen(m_server_socket, MAX_LISTEN);
    if(-1 == res)
    {
        printf("listen error:%m\n");
        return false;
    }
    //创建线程接收socket连接
    if(0 != pthread_create(&m_pidAccept, NULL, AcceptThread, this))
    {
        printf("create accept thread failed:%m\n");
        return false;
    }
    //创建管理线程
    if(0 != pthread_create(&m_pidManage, NULL, ManageThread, this))
    {
        printf("create manage thread failed:%m\n");
        return false;
    }
    return true;
}




//接收socket连接线程
void * TcpServer::AcceptThread(void * pParam)
{
    if(!pParam)
    {
        printf("param is null\n");
        return 0;
    }
    TcpServer * pThis = (TcpServer*)pParam;
    int nMax_fd = 0;
    int i = 0;
    while(1)
    {
        FD_ZERO(&pThis->m_fdReads);
        //把服务器监听的socket添加到监听的文件描述符集合
        FD_SET(pThis->m_server_socket, &pThis->m_fdReads);
        //设置监听的最大文件描述符
        nMax_fd = nMax_fd > pThis->m_server_socket ? nMax_fd : pThis->m_server_socket;
        std::map<int,ServerData>::iterator iter = pThis->m_client.begin();
        //把客户端对应的socket添加到监听的文件描述符集合
        for(; iter != pThis->m_client.end(); ++iter)
        {
            FD_SET(iter->first, &pThis->m_fdReads);
        }
        //判断最大的文件描述符
        if(!pThis->m_client.empty())
        {
            --iter;
            nMax_fd = nMax_fd > (iter->first) ? nMax_fd : (iter->first);
        }
        //调用select监听所有文件描述符
        int res = select(nMax_fd + 1, &pThis->m_fdReads, 0, 0, NULL);
        if(-1 == res)
        {
            printf("select error:%m\n");
            continue;
        }
        printf("select success\n");
        //判断服务器socket是否可读
        if(FD_ISSET(pThis->m_server_socket, &pThis->m_fdReads))
        {
            //接收新的连接
            int fd = accept(pThis->m_server_socket, 0,0);
            if(-1 == fd)
            {
                printf("accept error:%m\n");
                continue;
            }
            //添加新连接的客户
            ServerData sd = {0};
            sd.sockfd = fd;
            sd.remainDataLen = -1;
            pThis->m_client.insert(std::make_pair(fd,sd));
            printf("连接成功\n");
        }
        for(iter = pThis->m_client.begin(); iter != pThis->m_client.end();)
        {
            int plusFlag = 1;
            //判断客户是否可读
            if(-1 != (iter->first) && FD_ISSET((iter->first), &pThis->m_fdReads))
            {
                //一次recv是不现实的处理可能的分包现象,所以将原来的set<sockfd>改为 map<sockfd,serverData>
                bool res = pThis->RecvData(&(iter->second));
                printf("remainDataLen:%d data:%s",(iter->second).remainDataLen,(iter->second).data);
                if (!res || (res && (iter->second).remainDataLen==0 ) )
                {
                    //客户端已读完
                   pthread_mutex_lock(&pThis->m_mutex);
                   pThis->m_data.push_back(iter->second);
                   pthread_mutex_unlock(&pThis->m_mutex);
                   plusFlag = 0;
                   pThis->m_client.erase(iter++);
                   printf("读取完毕\n");
                }
            }
            if (plusFlag) ++iter;
        }
    }
}


bool TcpServer::RecvData(ServerData * sd)
{
  char buf[BUF_SIZE];
  int t;
  bzero(buf,sizeof buf);
  if ( sd->remainDataLen == -1){
    if ( 0 > recv(sd->sockfd,buf,10,0) )
    { //规约客户端传来的前10个字符表示数据包长度
      printf("RecvData error.%d %m",sd->sockfd);
      strcpy(sd->data,"RecvData error.");
      sd->remainDataLen = -2;
      return false;
    }
    int n;
    if (sscanf(buf,"%d",&n)!=1){
      printf("can not read data length!");
      strcpy(sd->data,"1|can not read data length!");
      sd->remainDataLen = -2;
      return false;
    }
    if (n>MAX_RECV_DATA_LEN){
      printf("data length exceed limit");
      strcpy(sd->data,"1|data length exceed limit");
      sd->remainDataLen = -2;
      return false;
    }
    sd->remainDataLen = n;
  }else {
    t = sd->remainDataLen>BUF_SIZE?BUF_SIZE:sd->remainDataLen;
    if ( 0 > recv(sd->sockfd,buf,sizeof buf,0) )
    {
      printf("RecvData error.%d %m",sd->sockfd);
      strcpy(sd->data,"1|RecvData error.");
      return false;
    }
    sd->remainDataLen -= t;
    strcat((char*)sd->data,buf);//(char*) ok???
  }
  return true;
}

//发送数据到指定的socket
bool TcpServer::SendData(const char * buf, size_t len, int sockfd)
{
    if(NULL == buf)
    {
        return false;
    }
    printf("开始发送\n");
    int res = send(sockfd, buf, len, 0);
    printf("发送完毕\n");
    if(-1 == res)
    {
        printf("send error:%m\n");
        return false;
    }
    return true;
}

//管理线程，用于创建处理线程
void * TcpServer::ManageThread(void * pParam)
{
    printf("ManageThread线程开始执行\n");
    if(!pParam)
    {
        return 0;
    }
    pthread_t pid;
    TcpServer * pThis = (TcpServer *)pParam;
    while(1)
    {
        //使用互斥量
        pthread_mutex_lock(&pThis->m_mutex);
        int nCount = pThis->m_data.size();
        pthread_mutex_unlock(&pThis->m_mutex);
        if(nCount > 0)
        {
            pid = 0;
            printf("创建处理线程\n");
            //创建处理线程
            if( 0 != pthread_create(&pid, NULL, OperatorThread, pParam))
            {
                printf("creae operator thread failed\n");
            }
        }
        //防止抢占CPU资源
        usleep(100);//u表示微秒
    }
}

//数据处理线程
void * TcpServer::OperatorThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }
    TcpServer * pThis = (TcpServer*)pParam;
    pthread_mutex_lock(&pThis->m_mutex);
    if(!pThis->m_data.empty())
    {
        //每次只处理一个ServerData
        ServerData data = pThis->m_data.front();
        pThis->m_data.pop_front();
        if(pThis->m_operaFunc)
        {
            //把数据交给回调函数处理
            pThis->m_operaFunc(data);
        }
    }
    pthread_mutex_unlock(&pThis->m_mutex);
    return NULL;
}

bool TcpServer::UnInitialize()
{
    //close 是关闭文件
    close(m_server_socket);
    for(std::map<int,ServerData>::iterator iter = m_client.begin(); iter != m_client.end(); ++iter)
    {
        close(iter->first);
    }
    m_client.clear();
    if(0 != m_pidAccept)
    {
        pthread_cancel(m_pidAccept);
    }
    if(0 != m_pidManage)
    {
        pthread_cancel(m_pidManage);
    }
    return true;
}
