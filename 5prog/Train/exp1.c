#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

int main ()
{
    int semid = semget (142, 1, 0666|IPC_CREAT);
    if (semid == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    struct sembuf semops[2] =
    { {0, -1, IPC_NOWAIT|SEM_UNDO},
      {0, 1, IPC_NOWAIT|SEM_UNDO} };

    if (semop(semid, semops, 2) == -1)
    {
        perror("semop");
    }

    return 0;
}
