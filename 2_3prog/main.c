#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "sosoket"
#define LISTEN_BACKLOG 255

typedef double T;
T const LEFT = 0.0;
T const RIGHT = 500.0;
unsigned long const NUM_OF_INTERVALS = 10000000L;

struct Data
{
    T left;
    T right;
    T sum;
};

T func (T x)
{
    return (sin(x));
}

ssize_t send_err(int sockfd, const void* buf, size_t len, int flags)
{
    int rtr_val = send(sockfd, buf, len, flags);
    if (rtr_val == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

ssize_t recv_err(int sockfd, void* buf, size_t len, int flags)
{
    int rtr_val = recv(sockfd, buf, len, flags);
    if (rtr_val == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int connect_err(int sockfd, struct sockaddr* addr, socklen_t addrlen)
{
    int rtr_val = connect(sockfd, addr, addrlen);
    if (rtr_val == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int accept_err(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    int rtr_val = accept(sockfd, addr, addrlen);
    if (rtr_val == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int listen_err(int sockfd, int backlog)
{
    int rtr_val = listen(sockfd, backlog);
    if (rtr_val == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int socket_err(int domain, int type, int protocol)
{
    int rtr_val = socket(domain, type, protocol);
    if (rtr_val == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

int bind_err(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    int rtr_val = bind(sockfd, addr, addrlen);
    if (rtr_val == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return rtr_val;
}

void job_giver_work()
{
    printf("I am giving job...\n");

    int local = 0, remote = 0;
    local = socket_err(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un local_ad, remote_ad;
    local_ad.sun_family = AF_UNIX;
    strcpy(local_ad.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH);
    int len = strlen(local_ad.sun_path) + sizeof(local_ad.sun_family);
    bind_err(local, (struct sockaddr *)&local_ad, len);

    listen_err(local, LISTEN_BACKLOG);

    while(1)
    {
        printf("I am waiting of connecting of some dick...\n");

        int remote_len = sizeof(remote_ad);
        remote = accept_err(local, (struct sockaddr *)&remote_ad, &remote_len);

        printf("Dick is connected...\n");

        struct Data job;
        job.left = LEFT;
        job.right = RIGHT;
        job.sum = 1.0;
        send_err(remote, &job, sizeof(job), 0);

        printf("Dick is counting sum...\n");

        recv_err(remote, &job, sizeof(job), 0);

        printf("Dick counted %lg\n", job.sum);
    }
}

void job_executer_work()
{
    printf("I am executing job...\n");

    int remote = socket_err(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un remote_ad;
    remote_ad.sun_family = AF_UNIX;
    strcpy(remote_ad.sun_path, SOCKET_PATH);
    int len = strlen(remote_ad.sun_path) + sizeof(remote_ad.sun_family);
    connect_err(remote, (struct sockaddr *)&remote_ad, len);

    struct Data job;
    recv_err(remote, &job, sizeof(job), 0);

    unsigned long const iters = NUM_OF_INTERVALS;
    T const step = (job.right - job.left)/iters;
    job.sum = 0.0;
    for (unsigned long i = 0; i < iters; ++i)
    {
        job.sum += func(job.left+i*step)*step;
    }

    printf("I counted %lg\n", job.sum);

    send_err(remote, &job, sizeof(job), 0);
}

int main(int argc, char** argv)
{
    if (argc == 1) //Job giver
    {
        job_giver_work();
    }
    else if (argc == 2) //Job executer
    {
        job_executer_work();
    }
    else
    {
        printf("Usage: for job    giver: %s\n", argv[0]);
        printf("     : for job executer: %s NUM_OF_CPUS\n", argv[0]);
    }
    return 0;
}
