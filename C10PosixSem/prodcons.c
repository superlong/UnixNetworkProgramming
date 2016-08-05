#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define NBUFF 100

struct {
	int		buff[NBUFF];
	sem_t	*mutex;
	sem_t	*nempty; /*number of empty buffer*/
	sem_t	*nstored; /*number of stored buffer*/
} shared;

void	*produce(void *);
void 	*consume(void *);


int main(int argc, char** argv)
{
	pthread_t tid_produce, tid_consume;

	if (argc != 2) {
		printf("usage: prodcons <nitems>");
		exit(-1);
	}
	
	int nitems = atoi(argv[1]);
	
	/*if there alread has the below semarphore,unlink*/	
	sem_unlink("mutex");
	sem_unlink("nempty");
	sem_unlink("nstored");

	/*create semaphore*/
	shared.mutex = sem_open("mutex", O_CREAT | O_EXCL, 0664, 1);
	if (SEM_FAILED == shared.mutex) {
		printf("sem_open failed, mutex\n");
		sem_unlink("mutex");
	}
	shared.nempty = sem_open("nempty", O_CREAT | O_EXCL, 0664, NBUFF);
	if (SEM_FAILED == shared.nempty) {
		printf("sem_open failed, nempty\n");
		sem_unlink("nempty");
	}
	shared.nstored = sem_open("nstored", O_CREAT | O_EXCL, 0664, 0);
	if (SEM_FAILED == shared.nstored) {
		printf("sem_open failed, nstored\n");
		sem_unlink("nstored");
	}
	
	setbuf(stdout, NULL);
	//pthread_setconcurrency(2);
	pthread_create(&tid_produce, NULL, produce, &nitems);
	pthread_create(&tid_consume, NULL, consume, &nitems);

	pthread_join(tid_produce, NULL);
	pthread_join(tid_consume, NULL);

	sem_unlink("mutex");
	sem_unlink("nempty");
	sem_unlink("nstored");
	
	exit(0);

}

void *produce(void* arg)
{
	int i, n = *(int *)arg;
	for (i = 0; i < n; ++i) {
		sem_wait(shared.nempty);
		//printf("prod: wait nempty = %d\n", *shared.nempty);
		sem_wait(shared.mutex);
		//printf("prod: wait mutex = %d\n", *shared.mutex);
		//sleep(1);	
		shared.buff[i % NBUFF] = i;

		{
			printf("	produce: buff[%d] = %d\n", i%NBUFF, shared.buff[i%NBUFF]);
		}
		sem_post(shared.mutex);
		//printf("prod: post mutex = %d\n", *shared.mutex);
		sem_post(shared.nstored);
		//printf("prod: post nstored = %d\n", *shared.nstored);
	}
	return NULL;
}


void* consume(void* arg)
{
	int i, n = *(int *)arg;
	for (i = 0; i < n; ++i) {
		sem_wait(shared.nstored);
		//printf("consumer: wait nstored = %d\n", *shared.nstored);
		sem_wait(shared.mutex);
		//printf("conusume: wait mutex = %d\n", *shared.mutex);
		if (shared.buff[i % NBUFF] != i) {
			printf("error: buff[%d] = %d", i%NBUFF, shared.buff[i%NBUFF]);
		}
		else {
			printf("	consume:buff[%d] = %d\n", i%NBUFF, shared.buff[i%NBUFF]);
		}
		sem_post(shared.mutex);
		//printf("conusume: post mutex = %d\n", *shared.mutex);
		sem_post(shared.nempty);	
		//printf("conusume: post nempty = %d\n", *shared.nempty);

	}
	return NULL;
}



