#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#define SEN_REC_REG 0
#define EX_SEN 1
#define EX_REC 2

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

void semop_err(int semid, struct sembuf* sops, unsigned nsops)
{
    if (nsops == 0)
    {
        return;
    }
    if ( semop(semid, sops, nsops) == -1)
    {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

void sender_work(char** argv)
{
    key_t key;
    int semid;
    int shmemid;

    //grabbing key 
    if ((key = ftok("init.c", 'R')) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //create a set with 3 semaphores
    if ((semid = semget(key, 3, 0666 | IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    struct sembuf semops_init[4] =
    { {SEN_REC_REG, 0, IPC_NOWAIT},
      {EX_SEN, 0, IPC_NOWAIT},
      {EX_REC, 0, IPC_NOWAIT}, 
      {EX_SEN, 1, SEM_UNDO}
    };
    semop_err(semid, semops_init, 4);

    //initializing  shared memory (1K)
    if ((shmemid = shmget(key, 1024, 0666|IPC_CREAT)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //getting adress of shared memory
    char* shmemp = shmat(shmemid, 0, 0);

    //opening file
    int fd = open (argv[1], 0444);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    //sending chunks
    //there're 0th, 1st and (2nd, 3rd, 4th...) iterations
    unsigned long i = 0;
    for (i = 0; 1; (i<2)?++i:i) //i==0 first and only first iteration 
                                //i==1 second and only second iteration
    {
        struct sembuf semops3[7] =
        { {EX_REC, -1, IPC_NOWAIT},
          {EX_REC, 1, IPC_NOWAIT},
          {SEN_REC_REG, 0, 0},
          {SEN_REC_REG, 1, 0},
          {SEN_REC_REG, -1, SEM_UNDO},
          {EX_REC, -1, IPC_NOWAIT}, 
          {EX_REC, 1, IPC_NOWAIT} };

        if (i==0) //first iteration
        {
            semop_err(semid, semops3+2, 1);
        }
        else if (i==1) //second iteration
        {
            semop_err(semid, semops3+2, 5);
        }
        else
        {
            semop_err(semid, semops3, 7);
        }

        char buffer[1024];
        int bytes_read = read (fd, &buffer, 1024-sizeof(unsigned long));
        if (bytes_read == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        memcpy (shmemp+sizeof(unsigned long), buffer, bytes_read);
        
        //sending info about read bytes 
        *((unsigned long*)shmemp) = bytes_read;

        struct sembuf semops7[3] =
        { {SEN_REC_REG, 1, SEM_UNDO},
          {EX_REC, -1, IPC_NOWAIT},
          {EX_REC, 1, IPC_NOWAIT} };

        //i==0 first and only first iteration
        semop_err(semid, semops7, (i==0)?1:3);
        
        if (bytes_read == 0)
        {
            break;
        }
    }
    

    //closing file
    close(fd);

    //detaching of shared memory
    shmdt (shmemp);
    
    fprintf(stderr, "Sender\'s work is done\n");
}

void receiver_work(char** argv)
{
    key_t key;
    int semid;
    int shmemid;

    //grabbing key 
    if ((key = ftok("init.c", 'R')) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //grabbing set of 3 semaphores
    if ((semid = semget(key, 3, 0666)) == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //entering through semaphores
    struct sembuf semops[4] =
    { {EX_REC, 0, IPC_NOWAIT}, //there're no other receivers
      {EX_SEN, -1, IPC_NOWAIT},
      {EX_SEN, 1, IPC_NOWAIT},
      {EX_REC, 1, SEM_UNDO} };
    semop_err(semid, semops, 4);

    //grabbing shared memory (1K)
    if ((shmemid = shmget(key, 1024, 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //getting adress of shared memory
    char* shmemp = shmat(shmemid, 0, 0);

    //receiving chunks
    //there're 0th, 1st and (2nd, 3rd, 4th...) iterations
    unsigned long i = 0;
    for (i = 0; 1; i<1?++i:i) //i==0 first and only first time
    {
        struct sembuf semops4[4] =
        { {EX_SEN, -1, IPC_NOWAIT},
          {EX_SEN, 1, IPC_NOWAIT},
          {SEN_REC_REG, -1, 0}, 
          {SEN_REC_REG, 1, SEM_UNDO} };
        semop_err(semid, semops4, 4);

        //receiving info about read bytes 
        unsigned long bytes_read = *((unsigned long*)shmemp);

        if (bytes_read == 0)
        {
            break;
        }

        char buffer[1024];
        memcpy (buffer, shmemp+sizeof(unsigned long), bytes_read);
        write(STDOUT_FILENO, buffer, bytes_read);

        struct sembuf semops7[3] =
        { {EX_SEN, -1, IPC_NOWAIT},
          {EX_SEN, 1, IPC_NOWAIT},
          {SEN_REC_REG, -1, SEM_UNDO} };
        semop_err(semid, semops7, 3);

        if (bytes_read == 0)
        {
            break;
        }
    }

    //detaching shared memory
    shmdt(shmemp);

    //deleting shared memory
    shmctl(shmemid, IPC_RMID, NULL);

    //deleting semaphore set
    semctl(semid, 5, IPC_RMID);

    fprintf (stderr, "Receiver\'s work is done\n");
}

int main (int argc, char** argv)
{
    errno = 0;
    if (argc == 2)
    {
        sender_work(argv);
    }
    else if (argc == 1)
    {
        receiver_work(argv);
    }
    else
    {
        printf ("Wrong num of arguments: must be 0 or 1, but there\'re %i\n",
            argc - 1);
    }
    return 0;
}
