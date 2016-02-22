#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char** argv)
{
    errno = 0;
    if (argc != 2)
    {
        printf("Wrong num of arguments. (There\'re %i args, must be 1)\n", argc - 1);
        assert(NULL);
    }

    char* endptr = NULL;
    long num_of_processes = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        printf("The argument is not a number.\n");
        assert(NULL);
    }
    if (errno == ERANGE) 
    {
        printf("The number is too big.\n");
        assert(NULL);
    }
    if (num_of_processes <= 0)
    {
        printf("Num of processes must be positive\n");
        assert(NULL);
    }
    
    
    
    
    pid_t pid = 1;
    int i = 0;
    for (i = 0; i < num_of_processes && pid != 0; ++i)
    {
        pid = fork();
        if (pid == 0)
        {
            printf ("N: %i, pid: %i, ppid: %i\n", i, getpid(), getppid());
        }
        else if (pid > 0)
        {
            int status = 0;
            wait (&status);
        }
        else
        {
            printf ("Error in forking.\n");
        }
    }
    return 0;
}
