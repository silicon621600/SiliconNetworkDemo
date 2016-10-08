#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
const int N = 1000;
void* BaiJi( void* arg )
{
        system("./client 0000000010A123456789");
        return NULL;
}


int main()
{

        pthread_t tidArry[N];
        pthread_attr_t attr;

        pthread_attr_init( &attr );
        pthread_attr_setstacksize( &attr, 1024 );

        int i;

        for( i =0; i<N; i++ )
        {
                if( pthread_create( &tidArry[i], &attr, BaiJi, NULL ) )
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
