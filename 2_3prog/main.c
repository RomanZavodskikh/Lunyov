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
#define MSG_BOSS "I am a boss"

typedef double T;
T const LEFT = 0.0;
T const RIGHT = 500.0;
unsigned long const NUM_OF_INTERVALS = 1000000000L;

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

void receive_msg(int i, int* sockfds2, fd_set* to_check, fd_set* not_readen,
    struct Data* jobs)
{
    if ( FD_ISSET(sockfds2[i], to_check) )
    {
        int numbytes = 0;

        if ( (numbytes = recv(sockfds2[i], &jobs[i],
            sizeof(jobs[i]), 0)) == -1)
        {
            perror("server: recv");
            exit(1);
        }
        if ( numbytes == 0 )
        {
            fprintf(stderr, "server: connection cutted\n");
            exit(1);
        }

        FD_CLR(sockfds2[i], not_readen);
        printf ("server: Received sum == %lg\n", jobs[i].sum);
    }
}

int fd_empty(fd_set* fds, int max_fd)
{
    for (int i = 0; i < max_fd+1; ++i)
    {
        if (FD_ISSET(i, fds))
        {
            return 0;
        }
    }
    return 1;
}

void set_socket_reuse(int sockfd)
{
    int yes = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))
        == -1)
    {
        perror("\tsetsockopt");
        exit(1);
    }
}

void set_socket_broadcast(int sockfd)
{
    int yes = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes))
        == -1)
    {
        perror("\tsetsockopt");
        exit(1);
    }
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
    printf("    %s: %s, protocol: %i\n", ipver, ipstr, p->ai_protocol);
}

void server(int argc, char** argv)
{
    char* endptr;
    unsigned long const max_slaves = strtoul(argv[2], &endptr, 10);
    if ( *endptr != '\0')
    {
        perror("server: strtoul");
        exit(1);
    }

    struct addrinfo hints, *servinfo_udp, *p_udp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rv;
    if ( (rv = getaddrinfo("255.255.255.255", PORT, &hints,
        &servinfo_udp)) != 0 )
    {
        fprintf(stderr, "server: getaddrinfo %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd_udp;
    for (p_udp = servinfo_udp; p_udp != NULL; p_udp = p_udp->ai_next)
    {
        if ( (sockfd_udp = socket(p_udp->ai_family, p_udp->ai_socktype,
            p_udp->ai_protocol)) == -1 )
        {
            perror("server: socket");
            continue;
        }

        set_socket_reuse(sockfd_udp);
        set_socket_broadcast(sockfd_udp);

        print_ip(p_udp);

        break;
    }

    if (p_udp == NULL)
    {
        fprintf(stderr, "server: failed to create socket\n");
        exit(1);
    }

    struct addrinfo *p, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ( (rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) == -1)
    {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ( (sockfd = socket(p->ai_family, 
            p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        set_socket_reuse(sockfd);
        
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

    if ( sendto(sockfd_udp, MSG_BOSS, strlen(MSG_BOSS)+1, 0,
        p_udp->ai_addr, p_udp->ai_addrlen) == -1)
    {
        perror("server: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo_udp);
    servinfo_udp = NULL;

    printf("server: sended broadcast '%s'\n", MSG_BOSS);

    int* sockfds2 = calloc(max_slaves, sizeof(*sockfds2));
    struct Data* jobs = calloc(max_slaves, sizeof(*jobs));
    struct sockaddr_storage their_addr;
    socklen_t their_size = 0;
    for (unsigned long i = 0; i < max_slaves; ++i)
    {
        jobs[i].left = LEFT + i*(RIGHT-LEFT)/max_slaves;
        jobs[i].right = LEFT + (i+1)*(RIGHT-LEFT)/max_slaves;
        jobs[i].sum = 0;
        jobs[i].intervals = NUM_OF_INTERVALS/max_slaves;


        if ( (sockfds2[i] = accept(sockfd, (struct sockaddr*)&their_addr,
            &their_size)) == -1)
        {
            perror("server: accept");
            exit(1);
        }

        printf("server: accept completed\n");
        
        if ( send(sockfds2[i], &jobs[i], sizeof(jobs[i]), 0) == -1)
        {
            perror("server: send");
            exit(1);
        }

        printf ("server: Sended (%lg, %lg, %li)!\n", jobs[i].left,
            jobs[i].right, jobs[i].intervals);
    }

    fd_set not_readen;
    fd_set to_check;
    FD_ZERO(&not_readen);
    FD_ZERO(&to_check);
    int max_fd = 0;
    for (unsigned long i = 0; i < max_slaves; ++i)
    {
        FD_SET(sockfds2[i], &not_readen);
        FD_SET(sockfds2[i], &to_check);

        if (sockfds2[i] > max_fd)
        {
            max_fd = sockfds2[i];
        }
    }

    while ( !fd_empty(&to_check, max_fd) ) 
    {
        if ( select(max_fd+1, &to_check, NULL, NULL, NULL) == -1)
        {
            perror("server: select");
            exit(1);
        }

        for (unsigned long i = 0; i < max_slaves; ++i)
        {
            receive_msg(i, sockfds2, &to_check, &not_readen, jobs);
        }

        to_check = not_readen;
    }
    free(sockfds2);
    free(jobs);

    T sum = 0.0;
    for (unsigned long i = 0; i < max_slaves; ++i)
    { 
        sum += jobs[i].sum;
    }
    printf("server: Integral == %lg\n", sum);
}

void client(int argc, char** argv)
{
    struct addrinfo hints, *p, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = 0;
    if ( (rv=getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ( (sockfd=socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        set_socket_reuse(sockfd);

        if ( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: bind");
            close(sockfd);
            continue;
        }

        print_ip(p);
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    servinfo = NULL;

    printf("client: binded\n");

    struct sockaddr_storage sa;
    socklen_t addrlen;
    char buf[20];
    
    if ( recvfrom(sockfd, buf, sizeof(buf), 0,
        (struct sockaddr*)&sa, &addrlen) == -1)
    {
        perror("client: recvfrom");
        exit(1);
    }

    printf("client: reveived '%s'\n", buf);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = (struct sockaddr*)&sa;

    if ( (rv=getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
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

        set_socket_reuse(sockfd);

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
        server(argc, argv);
    }
    else if (argc == 2 && !strcmp(argv[1], "client")) //Job executer
    {
        client(argc, argv);
    }
    else
    {
        printf("Usage: for server: %s server MAX_OF_SLAVES\n", argv[0]);
        printf("     : for client: %s client \n", argv[0]);
    }
    return 0;
}
