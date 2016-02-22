#ifndef SYSCALL_ERR_H
#define SYSCALL_ERR_H

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
#endif //SYSCALL_ERR_H
