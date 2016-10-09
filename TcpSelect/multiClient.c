#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
const int N = 1000;
void* BaiJi( void* arg )
{
        char * cmd = (char*) arg;

        system(cmd);
        return NULL;
}


int main(int argc,char * argv[])
{
        if (argc<=1){
          printf("command param error\n");
          return 0;
        }
        pthread_t tidArry[N];
        pthread_attr_t attr;

        pthread_attr_init( &attr );
        pthread_attr_setstacksize( &attr, 1024 );

        int i;

        for( i =0; i<N; i++ )
        {
                if( pthread_create( &tidArry[i], &attr, BaiJi, argv[1]) )
                {
                        printf("error\n");
                }

        }

        for( i=0; i<N; i++ )
        {
                pthread_join( tidArry[i], NULL );
        }
        return 0;
}
