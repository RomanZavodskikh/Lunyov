#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define READ 0
#define WRITE 1

void sigpipe(int signum)
{
    printf("SIGPIPE\n");
}

int main ()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipefd");
        exit(EXIT_FAILURE);
    }
    close(pipefd[READ]);
    signal(SIGPIPE, sigpipe);
    if (write(pipefd[WRITE], "Hel", 4) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    sleep(100);
    return 0;
}
