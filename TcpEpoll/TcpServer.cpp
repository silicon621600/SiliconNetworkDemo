
#include "TcpServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

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
    //绑定socket
    int res = bind(m_server_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == res)
    {
        printf("bind error:%m\n");
        return false;
    }
    //设置socket非阻塞
    if (make_socket_non_blocking(m_server_socket)<0){
      return false;
    }
    //监听
    res = listen(m_server_socket, MAX_LISTEN);
    if(-1 == res)
    {
        printf("listen error:%m\n");
        return false;
    }
    //初始化epoll相关
    struct epoll_event event;
    efd = epoll_create1(0);// 0 is ok????
    if (efd==-1){
        perror("epoll_create1");
        return 0;
    }
    event.data.fd = m_server_socket;
    event.events = EPOLLIN | EPOLLET;// 读 和边缘模式
    int s = epoll_ctl(efd,EPOLL_CTL_ADD,m_server_socket,&event);
    if (s==-1){
        perror("epoll_ctl");
        return 0;
    }
    /* Buffer where events are returned */
    events = (struct epoll_event *)calloc(MAXEVENTS,sizeof event);


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

    time_t minStartTime = time(NULL);
    while (1)
    {
        int n,i;
        //进入epoll_wait前先关闭那些超时的连接（具体读入数据超时） 效率？？？
        if (difftime(time(NULL),minStartTime)>MAX_CONN_TIME)
	{           
		minStartTime = time(NULL);
		std::map<int,ServerData>::iterator iter;
	        for (iter=pThis->m_client.begin();iter!=pThis->m_client.end();){
		        int flag = 1;
        		double diffTime = difftime(time(NULL),iter->second.startTime);
        		if (diffTime>MAX_CONN_TIME)
        		{
        			char msg[255] = "1|read data timeout.";
        			pThis->SendData(msg,strlen(msg),iter->first);
        			if (close(iter->first)){
        				perror("close timeout connection error");
        			}
        			pThis->m_client.erase(iter++);
        		}else{
        			if (iter->second.startTime<minStartTime){
        				minStartTime = iter->second.startTime;
        			}
        			iter++;
        		}        	
       		 }
        }
        n = epoll_wait(pThis->efd,pThis->events,MAXEVENTS,20);//等待事件发生,20ms -1 表示阻塞
        for (i=0;i<n;i++)
        {
          printf("pThis->events[%d].data.fd: %d\n",i,pThis->events[i].data.fd);
          if ((pThis->events[i].events & EPOLLERR) ||
              (pThis->events[i].events & EPOLLHUP) ||
              (!(pThis->events[i].events & EPOLLIN)))
          {
           /* An error has occured on this fd, or the socket is not
              ready for reading (why were we notified then?) */
              fprintf (stderr, "epoll error\n");
              close (pThis->events[i].data.fd);//????????
              continue;
          } else if (pThis->m_server_socket == pThis->events[i].data.fd)
          {
            /* We have a notification on the listening socket, which
             means one or more incoming connections. */
             //新连接可能有一个或者多个
             while (1)
             {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
		  char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
		  int s;
		  struct epoll_event event;

                  in_len = sizeof in_addr;
                  infd = accept (pThis->m_server_socket, &in_addr, &in_len);
                  if (infd == -1)
                  {
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                      {
                          /* We have processed all incoming
                             connections. */
                          break;
                      }
                      else
                      {
                          perror ("accept");
                          break;
                      }
                  }
                  //打印客户端信息
                  s = getnameinfo (&in_addr, in_len,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
                  if (s == 0)
                  {
                      printf("Accepted connection on descriptor %d "
                             "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                  }

                  /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                  s = pThis->make_socket_non_blocking (infd);
                  if (s == -1)
                  {
                      return 0; //只结束当前线程
                  }

                  
                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl (pThis->efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                  {
                      perror ("epoll_ctl (pThis->efd, EPOLL_CTL_ADD, infd, &event)");
                      return 0;
                  }
                  //添加新连接的客户
                  ServerData sd = {0};
                  sd.sockfd = infd;
                  sd.remainDataLen = -1;
                  sd.startTime = time(NULL);
                  pThis->m_client.insert(std::make_pair(infd,sd));
              }
              continue;
          }else
          {
              /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
	

              std::map<int,ServerData>::iterator iter = pThis->m_client.find(pThis->events[i].data.fd);
              if (iter == pThis->m_client.end()){
                fprintf(stderr, "cannot find ServerData of %d\n", pThis->events[i].data.fd);//理论上应该不可能出现这种情况
                return 0;
              }
              bool res = pThis->RecvData(&(iter->second));

              if (!res || (res && (iter->second).remainDataLen==0 ) )
              {
                  //客户端读入出错 或者已读完
                 pthread_mutex_lock(&pThis->m_mutex);
                 pThis->m_data.push_back(iter->second);
                 pthread_mutex_unlock(&pThis->m_mutex);
                 //移出m_client map
                 pThis->m_client.erase(pThis->events[i].data.fd);
                 //使epoll放弃监听
                 int ret = epoll_ctl (pThis->efd, EPOLL_CTL_DEL, iter->first, &(pThis->events[i]));
                 if (ret){
                      perror ("epoll_ctl");
                 }
              }
          }
        }
    }
}


bool TcpServer::RecvData(ServerData * sd)
{
  int t;
  int count,buflen;
  char buf[BUF_SIZE*2],tempBuf[BUF_SIZE];
  printf("读时serverData: [%s] %d %d\n",sd->data,sd->remainDataLen,sd->sockfd);
  while (1){
  
  
    int flag=0;
    
    bzero(buf,sizeof buf);
    buflen = 0;
    //读出一个buf来
    while (1)
    {
    	count = recv(sd->sockfd,tempBuf,sizeof tempBuf,0);
    	if (count<0){
    		if (errno != EAGAIN){
    			//错误
    			perror("recv error");
    			flag = 1;
    			break;
    		}else{
	    		//正常 暂时没有数据了
    			flag = 2;
    			break;
    		}
    	}else if (count==0){
    		//到达文件末 客户端关闭了连接
    		flag = 3;
    		break;
    	}
    	strncpy(buf,tempBuf,count);
    	buflen += count;
    	if (buflen>BUF_SIZE){
    		//可以先处理了
    		break;
    	}
    }
   
    if (flag==1){
	    strcpy(sd->data,"1|read error!errno != EAGAIN");
            sd->remainDataLen = -2;
            return false;
    }
    for (int i=0;i<buflen;i++)
    	if (buf[i]=='\0') buf[i]=' ';//\0 替换成空格
    if (sd->remainDataLen == -1){
    	//还未确定数据长度
    	int l = strlen(sd->data);
    	if (l+buflen>=10){
    		strncat(sd->data,buf,10-l);
    		int n;
		if (sscanf(sd->data,"%d",&n)!=1){
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
		int tn = buflen-(10-l);
		int tt = tn>n?n:tn;
	      	strncpy(sd->data,buf+10-l,tt);
	      	sd->data[tt]=0;
	        sd->remainDataLen = n-tt;
    	}else{
    		//需要继续读
    		strncat(sd->data,buf,10-l);
    		if (flag==3)
    		{
    			strcpy(sd->data,"1|read error!socket closed");
		        sd->remainDataLen = -2;
            		return false;
    		}
    	}
    }else{
	int tt = buflen>sd->remainDataLen?sd->remainDataLen:buflen;
	strncat(sd->data,buf,tt);
	sd->data[tt+1]=0;
	sd->remainDataLen -= tt;
	if (sd->remainDataLen>0 && flag==3)    
	{
    		strcpy(sd->data,"1|read error!socket closed");
		sd->remainDataLen = -2;
            	return false;
    	}
    }
    if (flag==2 || flag==3){
    	break;
    }
  }
  return  true;
}

//发送数据到指定的socket
bool TcpServer::SendData(const char * buf, size_t len, int sockfd)
{
    if(NULL == buf)
    {
        return false;
    }
    printf("开始发送\n");
    //由于sockfd是非阻塞的，所以发送也必须改成循环发送
    int t ,s=0;
    while (1){
	t=send(sockfd, buf+s, len, 0);    
	if (t<0){
	    if (t==EINTR || t==EWOULDBLOCK || errno==EAGAIN){
	    	//正常
	    }else{
	    	 printf("send error:%m\n");
	    	 return false;
	    }
	}else if (t==0){
		//连接关闭
		printf("send. connection closed\n");
		return false;
	}else {
	   s+=t;
	   len-=t;
	   if (len==0){
	   	//发完了
	   	break;
	   }
	}
    }

    printf("发送完毕\n");
    
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
        printf("处理线程中serverData: [%s] %d %d\n",data.data,data.remainDataLen,data.sockfd);
        if(pThis->m_operaFunc)
        {
            //把数据交给回调函数处理
            pThis->m_operaFunc(data);
        }
    }
    pthread_mutex_unlock(&pThis->m_mutex);
    return NULL;
}
int TcpServer::make_socket_non_blocking(int sockfd)
{
  int flags, s;

  flags = fcntl (sockfd, F_GETFL, 0);//F_GETFL 取得文件描述符状态旗标，此旗标为open（）的参数flags。
  if (flags == -1)
    {
      perror ("fcntl(sockfd,F_GETFL)");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sockfd, F_SETFL, flags);//F_SETFL 设置文件描述符状态旗标，参数arg为新旗标，但只允许O_APPEND、O_NONBLOCK和O_ASYNC位的改变，其他位的改变将不受影响。
  if (s == -1)
    {
      perror ("fcntl (sockfd, F_SETFL, flags)");
      return -1;
    }

  return 0;
}
bool TcpServer::UnInitialize()
{
    free(events);
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
