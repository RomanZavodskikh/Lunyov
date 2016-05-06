#include <iostream>

#include <cmath>
#include <sys/sysinfo.h>
#include <pthread.h>

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

int pthread_mutex_lock_err(pthread_mutex_t* mutex)
{
    int rtr_val = pthread_mutex_lock(mutex);
    if (rtr_val != 0)
    {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int pthread_mutex_unlock_err(pthread_mutex_t* mutex)
{
    int rtr_val = pthread_mutex_unlock(mutex);
    if (rtr_val != 0)
    {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int pthread_cond_broadcast_err(pthread_cond_t* cond)
{
    int rtr_val = pthread_cond_broadcast(cond);
    if (rtr_val != 0)
    {
        perror("pthread_cond_broadcast");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int pthread_mutex_init_err(pthread_mutex_t* mutex, const pthread_mutexattr_t*
    attr)
{
    int rtr_val = pthread_mutex_init(mutex, attr);
    if (rtr_val != 0)
    {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int pthread_cond_init_err(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
    int rtr_val = pthread_cond_init(cond, attr);
    if (rtr_val != 0)
    {
        perror("pthread_cond_init");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

void* calloc_err (size_t nmemb, size_t size)
{
    void* rtr_val = calloc(nmemb, size);
    if (rtr_val == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}
#endif //SYSCALL_ERR_H


using namespace std;

typedef double T;
typedef T(*F_P)(T);

unsigned long const NUM_OF_INTERVALS = 589824000L;
T const LEFT = 0.0;
T const RIGHT = 500.0;

struct Data
{
    T left;
    T right;
    T* sum_p;
    unsigned long num_of_cpus;
    unsigned long cur_cpu;
};

T func (T x)
{
    return abs(sin(x));
}

void init_args(Data* const args, unsigned long const num_of_cpus)
{
    T const delta = RIGHT - LEFT;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        args[i].left = LEFT + i*delta/num_of_cpus;
        args[i].right = LEFT + (i+1)*delta/num_of_cpus;
        args[i].sum_p = new T;
        args[i].num_of_cpus = num_of_cpus;
        args[i].cur_cpu = i;
    }
}

void deinit_args(Data* const args, unsigned long const num_of_cpus)
{
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        delete args[i].sum_p;
    }
}

void* add_integral(void* args)
{
    T const left = ((Data*)args)->left;
    T const right = ((Data*)args)->right;
    T* const sum_p = ((Data*)args)->sum_p;
    unsigned long const cur_cpu = ((Data*)args)->cur_cpu;
    unsigned long const num_of_cpus = ((Data*)args)->num_of_cpus;
    unsigned long const iters = NUM_OF_INTERVALS / num_of_cpus;
    T const step = (right-left)/iters;

    T sum = 0;

    for (unsigned long i = 0; i < iters; ++i)
    {
        sum += func(left+i*step);
    }

    *sum_p = sum*step;

    return NULL;
}

T sum_sums(const Data* const args, unsigned long const num_of_cpus)
{
    T rtr_val = 0;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        rtr_val += *(args[i].sum_p);
    }
    return rtr_val;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Usage: ./a.out NUM_OF_CPUS" << endl;
        exit(EXIT_FAILURE);
    }
    unsigned long num_of_cpus = strtoul_err(argv[1], NULL, 10);
    num_of_cpus = min(num_of_cpus, (long unsigned)get_nprocs());

    pthread_t* threads = new pthread_t[num_of_cpus];
    Data* args = new Data[num_of_cpus];

    init_args(args, num_of_cpus);

    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_create_err(&threads[i], NULL, add_integral, &args[i]); 
    }
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_join_err(threads[i], NULL);
    }

    T const sum = sum_sums(args, num_of_cpus);
    cout << sum << endl;

    deinit_args(args, num_of_cpus);

    delete [] args;
    delete [] threads;

    return 0;
}
