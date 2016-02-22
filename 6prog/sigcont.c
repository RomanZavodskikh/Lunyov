#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

void handler (int signum)
{
    if (signum == SIGSTOP)
    {
        printf("SIGSTOP\n"); 
    }
    if (signum == SIGCONT)
    {
        printf("SIGCONT\n"); 
    }
}

int main()
{
    int pid = fork();
    if (pid > 0)
    {
        struct sigaction act;
        act.sa_handler = handler;
        sigaction(SIGSTOP, &act, NULL);
        sigaction(SIGCONT, &act, NULL);
        pause();
        pause();
    }
    else if (pid == 0)
    {
        struct sigaction act;
        act.sa_handler = handler;
        sigaction(SIGSTOP, &act, NULL);
        sigaction(SIGCONT, &act, NULL);
        pause();
        pause();
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}
