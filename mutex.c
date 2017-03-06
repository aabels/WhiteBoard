#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NUM_THREADS 10
pthread_mutex_t mutex;
pthread_t thread[NUM_THREADS];
int fd, id[NUM_THREADS];

int fib(int n){
    if (n == 0 | n == 1)
        return 1;
    return fib(n-1) + fib(n-2);
}
void * run_thread(void * arg){
    int * threadNum = (int *) arg;
    pthread_mutex_lock(&mutex);
    char buf[30];
    sprintf(buf,"Thread %d first message\n",*threadNum);
    write(fd,buf,strlen(buf));
    fib(35);
    sprintf(buf,"Thread %d second message\n",*threadNum);
    write(fd,buf,strlen(buf));
    pthread_mutex_unlock(&mutex);
    return (void *)0;
}

int main(){
    fd = open("thread_file",O_RDWR | O_CREAT, S_IRWXU);
    pthread_mutex_init(&mutex,NULL);
    int i;
    for (i = 0; i < NUM_THREADS; i++){
        id[i] = i;
        printf("Creating thread %d\n",i);
        pthread_create(&thread[i], NULL, run_thread, (void *) &id[i]);
    }
    for (i = 0; i < NUM_THREADS; i++){
        pthread_join(thread[i],NULL);
        printf("Joined thread %d\n",i);
    }
    close(fd);
    return 0;

}
