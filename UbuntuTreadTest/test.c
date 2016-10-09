#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void * foo(void * arg){
//	system(""); ubuntu下容易崩溃
}
int main(){
	pthread_t id;
	int i=0;
	while (1){
		if (  pthread_create(&id,NULL,foo,NULL) !=0){
			return ;
		}
		i++;
		printf("i=%d\n",i);
	}
	return 0;
}
