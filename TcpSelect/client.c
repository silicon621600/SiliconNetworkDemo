#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT            25555
#define SERVERHOST      "localhost"

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

  nbytes = write (filedes, msg, strlen(msg) + 1);
  printf("send data:%s\n",msg);
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

int main(int argc,char * argv[])
{

  int sock;
  struct sockaddr_in servername;

  if (argc<=1){
    printf("error command param.\n");
    return 0;
  }

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket (client)");
      exit (EXIT_FAILURE);
    }

  /* Connect to the server. */
  init_sockaddr (&servername, SERVERHOST, PORT);
  if (0 > connect (sock,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      exit (EXIT_FAILURE);
    }

  /* Send data to the server. */
  write_to_server (sock,argv[1]);
  recv_from_server(sock);
  close (sock);
  exit (EXIT_SUCCESS);
}
