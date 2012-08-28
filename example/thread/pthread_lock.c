#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int i;
pthread_rwlock_t       rwlock;
void test(int ij)
{
    int rc;
    rc = pthread_rwlock_wrlock(&rwlock);
    i++;
    if(i > 1){
	printf("count %d\n", ij);
	//exit(1);
    }
    i = i - 1;
    usleep(1);
    rc = pthread_rwlock_unlock(&rwlock);
}

void *rdlockThread(void *arg)
{

    int ij = *(int *)arg;
    while(1){
	int rc;
	rc = pthread_rwlock_wrlock(&rwlock);
	i++;
	if(i > 1){
	    printf("count %d\n", ij);
	    //exit(1);
	}
	i = i - 1;
	usleep(1);
	rc = pthread_rwlock_unlock(&rwlock);
    }
  return NULL;
}

void *wrlockThread(void *arg)
{
    int ij = *(int *)arg;
    while(1){
	int rc;
	rc = pthread_rwlock_wrlock(&rwlock);
	i++;
	if(i > 1){
	    printf("count %d\n", ij);
	    //exit(1);
	}
	i = i - 1;
	usleep(1);
	rc = pthread_rwlock_unlock(&rwlock);
    }

  return NULL;
}

int main(int argc, char **argv)
{
  int                   rc=0;
  pthread_t             thread, thread1;
  int ij=0;

  printf("init\n");
  //rc = pthread_rwlock_init(&rwlock, NULL);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  rc = pthread_create(&thread, NULL, wrlockThread, &ij);
  rc = pthread_create(&thread1, NULL, wrlockThread, &ij);

  rc = pthread_rwlock_destroy(&rwlock);

  while(1);
  printf("end\n");
  return 0;
}

