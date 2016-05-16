#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LISTEN_BACKLOG 255
#define PORT "25123"
#define MSG_SLAVE "I am a slave!!!"

typedef double T;
T const LEFT = 0.0;
T const RIGHT = 500.0;
unsigned long const NUM_OF_INTERVALS = 100000000L;

struct Data
{
    T left;
    T right;
    T sum;
    unsigned long intervals;
};

T func (T x)
{
    return fabs(sin(x));
}

void* get_in_addr(struct sockaddr* sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void print_ip(struct addrinfo* p)
{
    void* addr;
    char* ipver;
    char ipstr[INET6_ADDRSTRLEN];

    if (p->ai_family == AF_INET)
    {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        addr = &(ipv4->sin_addr);
        ipver = "IPv4";
    }
    else //IPv6
    {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
        addr = &(ipv6->sin6_addr);
        ipver = "IPv6";
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf("    %s: %s, protocal: %i\n", ipver, ipstr, p->ai_protocol);
}

void job_giver_work()
{
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ( (rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0 )
    {
        fprintf(stderr, "server: getaddrinfo %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                == -1 )
        {
            perror("server: socket");
            continue;
        }

        if ( (bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1)
        {
            perror("server: bind");
            close(sockfd);
            continue;
        }

        print_ip(p);

        break;
    }

    freeaddrinfo(servinfo);
    servinfo = NULL;

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    int optval = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, 
        sizeof(optval)) == -1)
    {
        perror("server: setsockopt");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    int received = 1;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    {
        char buf[100];
        printf("\trecvfrom args:\n");
        printf("\tsockfd %i\n", sockfd);
        printf("\tbuf %p\n", buf);
        printf("\tsizeof(buf) %li\n", sizeof(buf));
        printf("\t0\n");
        printf("\t(struct sockaddr*)&addr %p\n", (struct sockaddr*)&addr);
        printf("\t&addrlen %p\n", &addrlen);
        
        if ( (received = recvfrom(sockfd, buf, sizeof(buf), 0,
            (struct sockaddr*)&addr, &addrlen)) == -1)
        {
            perror("server: recvfrom");
            exit(1);
        }

        printf("server: Received '%s'!\n", buf);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = (struct sockaddr*)&addr;

    if ( (rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) == -1)
    {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ( (sockfd = socket(p->ai_family, 
            p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        
        if ( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server: bind");
            close(sockfd);
            continue;
        }
        
        print_ip(p);

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    servinfo = NULL;
    printf("server: bind completed\n");

    if ( listen(sockfd, LISTEN_BACKLOG) == -1)
    {
        perror("server: listen");
        exit(1);
    }
    
    printf("server: listen completed\n");

    int sockfd2 = 0;
    struct sockaddr_storage their_addr;
    socklen_t their_size = 0;
    {
        struct Data job;
        job.left = LEFT;
        job.right = RIGHT;
        job.sum = 0;
        job.intervals = NUM_OF_INTERVALS;


        if ( (sockfd2 = accept(sockfd, (struct sockaddr*)&their_addr,
            &their_size)) == -1)
        {
            perror("server: accept");
            exit(1);
        }

        printf("server: accept completed\n");
        
        if ( send(sockfd2, &job, sizeof(job), 0) == -1)
        {
            perror("server: send");
            exit(1);
        }

        printf ("server: Sended (%lg, %lg, %li)!\n", job.left, job.right,
            job.intervals);
    }

    {
        struct Data job;

        if ( recv(sockfd2, &job, sizeof(job), 0) == -1)
        {
            perror("server: recv");
            exit(1);
        }

        printf ("server: Received sum == %lg\n", job.sum);
    }
}

void job_executer_work(int argc, char** argv)
{
    struct addrinfo hints, *servinfo, *p;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int rv = 0;
    if ((rv = getaddrinfo(argv[2], PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd = 0;
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        print_ip(p);
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    char s[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),
        s, sizeof(s));

    freeaddrinfo(servinfo);
    servinfo = NULL;
    printf("client: connecting to %s\n", s);

    if ( sendto(sockfd, MSG_SLAVE, sizeof(MSG_SLAVE),  
            0, (struct sockaddr*)p->ai_addr, p->ai_addrlen) == -1)
    {
        perror("client: sendto");
        exit(1);
    }

    printf("client: Sended '%s'!\n", MSG_SLAVE);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ( (rv=getaddrinfo(argv[2], PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ( (sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if ( (connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1)
        {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        print_ip(p);
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    servinfo = NULL;
    printf("client: connection completed\n");

    struct Data job;
    if ( (recv(sockfd, &job, sizeof(job), 0)) == -1)
    {
        perror("client: recv");
        exit(1);
    }

    printf("client: Received (%lg, %lg, %li)!\n", job.left, job.right,
        job.intervals);

    unsigned long iters = job.intervals;
    T const step = (job.right-job.left)/iters;
    job.sum = 0;
    for(unsigned long i = 0; i < iters; ++i)
    {
        job.sum += func(job.left+i*step);
    }
    job.sum *= step;

    if ( (send(sockfd, &job, sizeof(job), 0)) == -1)
    {
        perror("client: send");
        exit(1);
    }

    printf("client: Sended sum == %lg\n", job.sum);

    close(sockfd);
}

int main(int argc, char** argv)
{
    setbuf(stdout, NULL);
    if (argc == 3 && !strcmp(argv[1], "server")) //Job giver
    {
        job_giver_work();
    }
    else if (argc == 3 && !strcmp(argv[1], "client")) //Job executer
    {
        job_executer_work(argc, argv);
    }
    else
    {
        printf("Usage: for job    giver: %s server MAX_OF_SLAVES\n", argv[0]);
        printf("     : for job executer: %s client BOSS_ADDR\n", argv[0]);
    }
    return 0;
}
