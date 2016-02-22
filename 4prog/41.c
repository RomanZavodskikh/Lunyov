#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MQ_FILENAME "MQ"

const char PROJ_ID = 'A';
const int ERROR_WAS = 1;
const int ERROR_WASNT = 2;

struct msg_type
{
    long type;
    long num_of_last;
};

void print_error (const char* msg)
{
    printf("%s; %s\n", msg, strerror(errno));
    exit(-1);
}

void print_n_nums (long n)
{
    pid_t pid = 1;
    long i = 0;
    int mq_id = 0;

    int mq_key = ftok(MQ_FILENAME, PROJ_ID);
    if ((mq_id = msgget (mq_key, 0644 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        print_error ("Couldn\'t get the message queue id");
    }

    for (i = 1; i <= n && pid != 0; ++i)
    {
        pid = fork();
        if (pid < 0)
        {
            printf ("Error in forking.\n");

            msgctl(mq_id, IPC_RMID, NULL);
            break;
        }
        else if (pid == 0)
        {
            break;
        }
    }

    if (pid == 0 && i == n) //the last child if success
    {
        struct msg_type message = {1, n};
        msgsnd(mq_id, &message, sizeof(message)-sizeof(long), 0);
    }

    if (pid == 0) //child
    {
        struct msg_type message;
        if ( (msgrcv (mq_id, &message, sizeof(message)-sizeof(long), i,
            MSG_NOERROR)) == -1)
        {
            exit(EXIT_FAILURE);
        }

        printf ("%li ", i);
        fflush(stdout);

        if ( i < message.num_of_last) //not last child
        {
            message.type++;
            if ( msgsnd (mq_id, &message,
                sizeof(message)-sizeof(long), 0) == -1)
            {
                print_error ("The msgsnd error in child process");
            }
        }

        if ( i == message.num_of_last)
        {
            msgctl(mq_id, IPC_RMID, NULL);
        }
    }
    if (pid > 0)
    {
    }
}

int main (int argc, char** argv)
{
    errno = 0;
    if (argc != 2)
    {
        printf("Wrong num of arguments. (There\'re %i args, must be 1)\n",
            argc - 1);
        exit(-1);
    }

    char* endptr = NULL;
    long num_of_processes = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        print_error ("The argument is not a number.");
    }
    if (errno == ERANGE) 
    {
        print_error("The number is too big.");
    }
    if (num_of_processes <= 0)
    {
        print_error("Num of processes must be positive");
    }
    print_n_nums (num_of_processes);
    return 0;
}
