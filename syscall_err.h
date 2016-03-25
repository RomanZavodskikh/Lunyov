#ifndef SYSCALL_ERR_H
#define SYSCALL_ERR_H

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

int fcntl_err(int fd, int cmd, int arg)
{
    int rtr_val = fcntl(fd, cmd, arg);
    if (rtr_val == -1)
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int select_err(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
    struct timeval* timeout)
{
    int rtr_val = select(nfds, readfds, writefds, exceptfds, timeout);
    if (rtr_val == -1)
    {
        perror("select");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int close_err(int fd)
{
    int rtr_val = close(fd);
    if (rtr_val == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int open_err(const char* pathname, int flags)
{
    int rtr_val = open(pathname, flags);
    if (rtr_val == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

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

int pipe_err(int pipefd[2])
{
    int rtr_val = pipe(pipefd);
    if (rtr_val == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

ssize_t read_err (int fd, void* buf, size_t count)
{
    ssize_t rtr_val = read(fd, buf, count);
    if(rtr_val == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

ssize_t write_err (int fd, void* buf, size_t count)
{
    ssize_t rtr_val = write(fd, buf, count);
    if(rtr_val == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

pid_t setsid_err(void)
{
    pid_t rtr_val = setsid();
    if(rtr_val == -1)
    {
        perror("setsid");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int ioctl_err(int d, int request, void* arg)
{
    int rtr_val = ioctl(d, request, arg);
    if (rtr_val == -1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int sigaction_err(int signum, const struct sigaction* act,
    struct sigaction* oldact)
{
    int rtr_val = sigaction(signum, act, oldact);
    if (rtr_val == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

unsigned long int strtoul_err(const char* nptr, char** endptr, int base)
{
    char* endptr_tmp = NULL;
    unsigned long int rtr_val = strtoul(nptr, &endptr_tmp, base);
    if (*nptr != '\0' && *endptr_tmp == '\0')
    {
        return rtr_val;
    }
    else
    {
        perror("strtoul");
        exit(EXIT_FAILURE);
    }
}

int pthread_create_err(pthread_t* thread, const pthread_attr_t* attr, 
    void*(*start_routine)(void*), void* arg)
{
    int rtr_val = pthread_create(thread, attr, start_routine, arg);
    if (rtr_val != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int pthread_join_err(pthread_t thread, void** retval)
{
    int rtr_val = pthread_join(thread, retval);
    if (rtr_val != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}
#endif //SYSCALL_ERR_H

