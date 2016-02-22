#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

pid_t pid;

void handler()
{
    printf("%d\n", pid);
    sleep(10);
    exit(0);
}

int main()
{
    signal(SIGCHLD, handler);
    pid = fork();
    wait(NULL);
    return 0;
}
