#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

#define   FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef struct shared {
     sem_t     mutex;
     int       cnt;
} shared;



int main(int argc, char** argv)
{
     int fd, i, nloop;
     shared s;
     s.cnt = 0;
     shared *ptr;
     if (argc != 3) {
          printf("usage: mmap [filepath] nloop\n");
          exit(-1);
     }

     nloop = atoi(argv[2]);
     fd = open(argv[1], O_RDWR | O_CREAT, FILE_MODE);
     write(fd, &s, sizeof(shared));
    
     ptr = mmap(NULL, sizeof(struct shared), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
     
     //ptr =  (shared *)malloc(sizeof(shared)); //do not work for IPC
     close(fd);
    
     sem_init(&ptr->mutex, 1, 1);
    
     setbuf(stdout, NULL);
     if (fork() == 0) {  /*child*/
          for (i = 0; i < nloop; i++) {
               sem_wait(&ptr->mutex);
               printf("child: %d\n", ptr->cnt++);
               sem_post(&ptr->mutex);
          }
          exit(0);
     }
    
     /*parent*/
     for (i = 0; i < nloop; i++) {
          sem_wait(&ptr->mutex);
          printf("parent: %d\n", ptr->cnt++);
          sem_post(&ptr->mutex);
     }
     exit(0);
}
