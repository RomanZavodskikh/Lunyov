#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig)
{
    printf("Not this time!\n");
}

int main (void)
{
    char s[200];

    //set up the handler
    if ( signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    printf ("Enter a string:\n");

    if (fork() == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (fgets(s, 200, stdin) == NULL)
    {
        perror("fgets");
    }
    else
    {
        printf("You entered: \"%s\"", s);
    }

    return 0;
}
