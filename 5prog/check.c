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

    if ((key = ftok("init.c", 'R')) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //create a set with 3 semaphores
    if ((semid = semget(key, 3, 0666)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    for (i = 0; i < 3; ++i)
    {
        printf ("Sem#%03i:%06i\n", i, semctl(semid, i, GETVAL));
    }

    return 0;
}
