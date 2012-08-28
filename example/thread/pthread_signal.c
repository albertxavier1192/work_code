/* gcc pthread_signa.c -lpthread */
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

pthread_t thread_t[3];

void sig_handler(int signo)
{
    printf("Thread Stop %d:\n",(int)pthread_self());
    while(1);
}

void null(int signo)
{
    printf("Thread Start\n");
}

void *test(void *data)
{
    sigset_t newmask;
    struct sigaction act, act2;
    int i = 0;

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGCONT);

    act.sa_handler = sig_handler;
    act2.sa_handler = null;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCONT, &act2, NULL);

    pthread_sigmask(SIG_UNBLOCK, &newmask, NULL);
    printf("hi\n");

    while(1) 
    {
	printf("running %d %d\n",i++, (int)pthread_self());
	sleep(1);
    }
}


void *worker(void *data)
{

    sigset_t newmask;

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGCONT);

    pthread_sigmask(SIG_UNBLOCK, &newmask, NULL);

    while(1)
    {
	sleep(2);
	pthread_kill(thread_t[0], SIGUSR1);
	pthread_kill(thread_t[1], SIGUSR1);
	sleep(3);
	pthread_kill(thread_t[0], SIGCONT);
	pthread_kill(thread_t[1], SIGCONT);
	sleep(10);
    }
}

int main()
{
    pthread_create(&thread_t[0], NULL, test, NULL);
    pthread_create(&thread_t[1], NULL, test, NULL);
    pthread_create(&thread_t[2], NULL, worker, NULL);

    pthread_join(thread_t[0], NULL);
    pthread_join(thread_t[1], NULL);
    pthread_join(thread_t[2], NULL);

    return 1;
}
