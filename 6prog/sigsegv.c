#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "../syscall_err.h"

void sigsegv_handler(int signum)
{
}

int main ()
{
    struct sigaction act;
    act.sa_handler = sigsegv_handler;
    sigaction(SIGSEGV, &act, NULL);

    char* null = 0;
    char a = *null;
    return 0;
}
