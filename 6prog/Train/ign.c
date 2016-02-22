#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int main()
{
    printf("Waiting for SIGUSR1...\n");

    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pause();
    printf("SIGUSR1 ignored\n");
    return 0;
}
