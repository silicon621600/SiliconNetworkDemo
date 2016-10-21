#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
int port  =    25555;

void init_sockaddr (struct sockaddr_in *name,
               const char *hostname,
               uint16_t port)
{
  struct hostent *hostinfo;

  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr, "Unknown host %s.\n", hostname);
      exit (EXIT_FAILURE);
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}

void write_to_server (int filedes,char* msg)
{
  int nbytes;
  int l = strlen(msg);
  nbytes = write (filedes, msg, l);
  printf("send data:[%s] %d\n",msg,l);
  if (nbytes < 0)
    {
      perror ("write");
      exit (EXIT_FAILURE);
    }
}
void recv_from_server(int sockfd)
{
  char ret[4096];
  char buf[256];
  int t;
  ret[0]=0;
  while (1){
    bzero(buf,sizeof buf);
    t = recv(sockfd,buf,sizeof buf,0);
    printf("t:%d [%s]\n",t,buf);
    if (t<0){
      perror("recv error");
      break;
    }else if (t==0){
      break;
    }else{
        strcat(ret,buf);
    }
  }
  printf("recv data:%s\n",ret);
}

void * deal(void * arg)
{
  char ** argv =(char **)arg;
  int sock;
  struct sockaddr_in servername;


  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket (client)");
        return NULL;
    }
  
  /* Connect to the server. */
  init_sockaddr (&servername, argv[1], port);
  if (0 > connect (sock,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      return NULL;
    }

  /* Send data to the server. */
  write_to_server (sock,argv[2]);
  recv_from_server(sock);
  close (sock);
  return NULL;
}
int main(int argc,char * argv[])
{
	int n =1;
	if (argc<3){
	  printf("command param must be this format: \"host datastring [threadNum=1]\"\n");
	  return 0;
	}
	if (argc>=4){
		port = atoi(argv[3]);
		if (n<=0 || n>65535)
		{
			printf("command param port error.\n");
		} 
	}
	if (argc>=5){
		n = atoi(argv[4]);
		if ( n<=0 || n>20000){
			printf("command param thread number error.\n");
			return ;
		}
	}
	pthread_t tidArry[20000];
	pthread_attr_t attr;

	pthread_attr_init( &attr );
	pthread_attr_setstacksize( &attr, 1024 );

	int i;

	for( i =0; i<n; i++ )
	{
		if( pthread_create( &tidArry[i], &attr, deal, argv) )
		{
		        perror("pthread_create error");
		}

	}

	for( i=0; i<n; i++ )
	{
		pthread_join( tidArry[i], NULL );
	}
	return 0;
}
