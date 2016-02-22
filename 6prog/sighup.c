#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int sigaction_err(int signum, const struct sigaction* act, struct sigaction*
    oldact)
{
    int rtr_val = sigaction(signum, act, oldact);
    if (rtr_val == -1)
    {
        fprintf(stderr, "sigaction(%d, %p, %p)\n", signum, act, oldact);
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

void handler (int signum)
{
    printf("%d\n", signum);
    if (signum == SIGALRM)
    {
        exit(EXIT_SUCCESS);
    }
}

int main ()
{
    struct sigaction act;
    act.sa_handler = handler;
    int i = 1;
    for (i = 1; i <= 31; ++i)
    {
        if (i == SIGKILL || i == SIGSTOP) continue;
        sigaction_err (i, &act, NULL);
    }
    while (1)
    {
        alarm(10);
        pause();
    }
    return 0;
}
