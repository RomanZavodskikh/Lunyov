#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h> 
#include <stdlib.h>
#include <stdio.h>

pid_t fork_err(void)
{
    pid_t rtr_val = fork();
    if (rtr_val == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

pid_t setsid_err(void)
{
    pid_t rtr_val = setsid();
    if (rtr_val == -1)
    {
        perror("setsid");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int kill_err(pid_t pid, int sig)
{
    int rtr_val = kill(pid, sig);
    if (rtr_val == -1)
    {
        perror("kill");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

pid_t setpgid_err (pid_t pid, pid_t pgid)
{
    pid_t rtr_val = setpgid(pid, pgid);
    if (rtr_val == -1)
    {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

static void sig_hup(int);
static void pr_ids(char*);

int main (void)
{
    char c;
    pid_t pid;

    pid = fork_err();
    if (pid != 0)
    {
        exit(0);
    }
    setsid_err();

    pid = fork_err();
    if (pid != 0)
    {
        pause();
    }
    setpgid_err(0, 0);

    pid = fork_err();
    if (pid == 0)
    {
        while (1)
        {
            pause();
        }
    }
    kill_err(pid, SIGSTOP);

    int i = 0;
    for (i = 0; i < 3; ++i)
    {
        fork_err();
    }
    
    while(1)
    {
        pause();
    }

    return 0;
}

static void sig_hup (int signo)
{
    printf("SIGHUP received, pid = %d\n", getpid());
}

static void pr_ids (char* name)
{
    printf("%s: pid=%d, ppid=%d, pgrp=%d\n", name, getpid(), getppid(),
        getpgrp());
    fflush(stdout);
}
