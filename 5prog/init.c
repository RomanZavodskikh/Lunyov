#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

int main (void)
{
    key_t key;
    int semid;
    int shmemid;
    union semun arg;

    if ((key = ftok("init.c", 'R')) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //create a set with 4 semaphores
    if ((semid = semget(key, 4, 0666 | IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //initalize semaphores #0, #2, #3 to 1
    arg.val = 1;
    int i = 0;
    for (i = 0; i < 4; ++i)
    {
        if (i == 1)
        {
            continue;
        }
        
        if (semctl(semid, i, SETVAL, arg) == -1)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }

    //initialize semaphore #1 to 0
    arg.val = 0;
    if (semctl(semid, 1, SETVAL, arg) == -1)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    //initializing  shared memory (1K)
    if ((shmemid = shmget(key, 1024, 0666|IPC_CREAT)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    return 0;
}
