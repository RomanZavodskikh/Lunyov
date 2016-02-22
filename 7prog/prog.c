#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define READ 0
#define WRITE 1

const int CH_BUF_SIZE = 4096;

const long BUFFER_SIZES[] =
{
    3L*4096L,
    9L*4096L,
    27L*4096L,
    81L*4096L,
    243L*4096L,
    729L*4096L,
    2187L*4096L,
    6561L*4096L,
    19683L*4096L,
    59049L*4096L,
    177147L*4096L,
    531441L*4096L,
    1594323L*4096L,
    4782969L*4096L,
    14384907L*4096L,
    43046721L*4096L,
    129140163L*4096L,
    387420489L*4096L,
    1162261467L*4096L,
    3486784401L*4096L
};

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

struct ring_buf
{
    //static characteristics
    char* buffer;
    char* eobuffer;
    unsigned long capacity;

    //dynamic characteristics
    char* start;
    char* end;
    int full;
};

ssize_t ring_buf_read(struct ring_buf* rb, int fd)
{
    //reads from ring buffer bytes and writes them to ->fd<- file
    unsigned long bytes_to_read = 0;
    unsigned long bytes_readen = 0;
    //fprintf(stderr, "RBR:%p...%p||s:%p<-->%p:e(full:%i)\n", rb->buffer,
    //    rb->eobuffer, rb->start, rb->end, rb->full);
    if (rb->start < rb->end)
    {
        bytes_to_read = rb->end - rb->start;
    }
    else if (rb->end < rb->start)
    {
        bytes_to_read = rb->eobuffer-rb->start;
    }
    else if (rb->full)
    {
        bytes_to_read = rb->eobuffer-rb->start;
    }
    else
    {
        //do nothing
    }
    
    bytes_to_read = (bytes_to_read>CH_BUF_SIZE)?CH_BUF_SIZE:bytes_to_read;
    bytes_readen = write_err(fd, rb->start, bytes_to_read);
    rb->start += bytes_readen;

    if (rb->start==rb->eobuffer)
    {
        rb->start = rb->buffer;
    }

    if (rb->full && bytes_readen != 0)
    {
        rb->full = 0;
    }

    return bytes_readen;
}

ssize_t ring_buf_write(struct ring_buf* rb, int fd)
{
    //writes to ring buffer bytes from ->fd<- file
    unsigned long bytes_to_write = 0;
    unsigned long bytes_written = 0;
    //fprintf(stderr, "RBW:%p...%p||s:%p<-->%p:e(full:%i)\n", rb->buffer,
    //    rb->eobuffer, rb->start, rb->end, rb->full);
    if (rb->start < rb->end)
    {
        bytes_to_write = rb->eobuffer - rb->end;
    }
    else if (rb->end < rb->start)
    {
        bytes_to_write = rb->start - rb->end;
    }
    else if (!rb->full)
    {
        bytes_to_write = rb->eobuffer - rb->end;
    }
    else
    {
        //do nothing
    }
    bytes_written = read_err(fd, rb->end, bytes_to_write);
    rb->end += bytes_written;

    if (rb->end == rb->eobuffer)
    {
        rb->end = rb->buffer;
    }

    if (rb->start == rb->end && bytes_written != 0)
    {
        rb->full = 1;
    }
    return bytes_written;
}

void ring_buf_ctor(struct ring_buf* rb, unsigned long cap)
{
    rb->buffer = calloc(cap, sizeof(char));
    rb->eobuffer = rb->buffer + cap;
    rb->capacity = cap;

    rb->start = rb->buffer;
    rb->end = rb->buffer;
    rb->full=0;
}

void ring_buf_dtor(struct ring_buf* rb)
{
    free(rb->buffer);
    rb->buffer=rb->eobuffer=rb->start=rb->end=NULL;
    rb->capacity=0;
    rb->full=0;
}

int** pipes = NULL;
long a = -1;
char* filename = NULL;
int child_that_finished = -1;

void child_work (unsigned num_of_child)
{
    //closing all useless fd-d (pipes)
    if (num_of_child == 0)
    {
        close_err(pipes[2*num_of_child][READ]);
        close_err(pipes[2*num_of_child][WRITE]);
        close_err(pipes[2*num_of_child+1][READ]);
    }
    else
    {
        close_err(pipes[2*num_of_child][WRITE]);
        close_err(pipes[2*num_of_child+1][READ]);
    }
    int i = 0;
    for (i = 0; i < 2*a; ++i)
    {
        if (i != 2*num_of_child && i != (2*num_of_child+1) )
        {
            close_err(pipes[i][READ]);
            close_err(pipes[i][WRITE]);
        }
    }

    //opening file
    int fd = -1;
    if (num_of_child == 0)
    {
        fd = open_err(filename, 0444);
    }

    //cycle of read-write
    char buffer[CH_BUF_SIZE];
    int readen = 1;
    while (readen!=0)
    {
        //read
        if (num_of_child == 0)
        {
            readen = read_err(fd, buffer, CH_BUF_SIZE);
        }
        else
        {
            readen = read_err(pipes[2*num_of_child][READ],
                buffer, CH_BUF_SIZE);
        }

        //write
        write_err(pipes[2*num_of_child+1][WRITE], buffer, readen);
    }

    //closing file (if opened) and all pipes
    if (num_of_child == 0)
    {
        close_err(pipes[2*num_of_child+1][WRITE]);
        close_err(fd);
    }
    else
    {
        close_err(pipes[2*num_of_child+1][WRITE]);
        close_err(pipes[2*num_of_child][READ]);
    }
}

void parallel_transfer(int i, struct ring_buf* rbuffers, fd_set* read_fds_p,
    fd_set* write_fds_p, int* was_reading, int* was_writing,
    fd_set* total_read_fds_p)
{
    //this function reads amount of data from i_th child (if it's possible)
    //(it's possible <=> its fd in *read_fds_p)
    //and the sends data to (i+1)_th child/STDOUT (if it's possible)
    //(it's possible <=> its fd in *write_fds_p OR it's STDOUT)
    int readen = 0;
    int written = 0;

    //read
    if (FD_ISSET(pipes[2*i+1][READ], read_fds_p) && !rbuffers[i].full)
    {
        readen = ring_buf_write(&rbuffers[i], pipes[2*i+1][READ]);
    }
    //if we read everything
    if (readen == 0 && FD_ISSET(pipes[2*i+1][READ], read_fds_p)
        && !rbuffers[i].full)
    {
        close_err(pipes[2*i+1][READ]);
        FD_CLR(pipes[2*i+1][READ], total_read_fds_p);
        child_that_finished = i;
        *was_reading = 1;
    }
    //if we have written everything
    //we never pass this "if" if we hasn't passed previous one
    if (readen == 0 && FD_ISSET(pipes[2*i+1][READ], read_fds_p)
        && i != (a-1) && rbuffers[i].start == rbuffers[i].end
        && !rbuffers[i].full && child_that_finished >= i)
    {
        close_err(pipes[2*i+2][WRITE]);
    }
    if (readen != 0 || rbuffers[i].full)
    {
        *was_reading = 1;
    }

    //write
    if(i != (a-1) && FD_ISSET(pipes[2*i+2][WRITE], write_fds_p))
    {
        written = ring_buf_read(&rbuffers[i], pipes[2*i+2][WRITE]);
    }
    if (i == a-1)
    {
        written = ring_buf_read(&rbuffers[a-1], STDOUT_FILENO);
    }
    //if we sended everything, but not in stdout but to some child
    if (i != (a-1) && FD_ISSET(pipes[2*i+2][WRITE], write_fds_p)
        && rbuffers[i].start == rbuffers[i].end && !rbuffers[i].full
        && child_that_finished >= i)
    {
        close_err(pipes[2*i+2][WRITE]);
    }
    if (written != 0)
    {
        *was_writing = 1;
    }
}

void set_read_fds(struct ring_buf* rbuffers, fd_set* read_fds_p)
{
    int i = 0;
    for (i = 0; i < a; ++i)
    {
        if (rbuffers[i].full)
        {
            FD_CLR(pipes[2*i+1][READ], read_fds_p);
        }
    }
}

void set_write_fds(struct ring_buf* rbuffers, fd_set* write_fds_p)
{
    int i = 0;
    FD_ZERO(write_fds_p);
    for (i = 0; i < a-1; ++i)
    {
        if (rbuffers[i].start != rbuffers[i].end || rbuffers[i].full)
        {
            FD_SET(pipes[2*i+2][WRITE], write_fds_p);
        }
    }
}

int fd_set_empty(int max_fd_plus_one, fd_set* fds)
{
    int i = 0;
    for (i = 0; i < max_fd_plus_one; ++i)
    {
        if (FD_ISSET(i, fds))
        {
            return 0;
        }
    }
    return 1;
}

void parent_work ()
{
    //closing all useless fd-s (pipes)
    int i = 0;
    close_err(pipes[0][WRITE]);
    for (i = 0; i < a; ++i)
    {
        close_err(pipes[2*i][READ]);
        close_err(pipes[2*i+1][WRITE]);
    }

    //initializing of ring buffers
    struct ring_buf rbuffers[a];
    for (i = 0; i < a; ++i)
    {
        ring_buf_ctor(&rbuffers[i], BUFFER_SIZES[a-i-1]);
    }

    //creating fd set of all read fd_s
    fd_set total_read_fds;
    FD_ZERO(&total_read_fds);
    for (i = 0; i < a; ++i)
    {
        FD_SET(pipes[2*i+1][READ], &total_read_fds);
    }

    //getting max_fd
    int max_fd = -1;
    for (i = 0; i < a; ++i)
    {
        if (pipes[2*i+1][READ] > max_fd)
        {
            max_fd = pipes[2*i+1][READ];    
        }
        if (i == a-1)
        {
            break;
        }
        if (pipes[2*i+2][WRITE] > max_fd)
        {
            max_fd = pipes[2*i+2][WRITE];
        }
    }

    //making IO nonblocking
    for (i = 0; i < a; ++i)
    {
        fcntl_err(pipes[2*i+1][READ], F_SETFD, 0666|O_NONBLOCK);
        if (i == a-1)
        {
            break;
        }
        fcntl_err(pipes[2*i+2][WRITE], F_SETFD, 0666|O_NONBLOCK);
    }

    //initializing of cycle data
    int was_reading = 1;
    int was_writing = 1;
    fd_set read_fds = total_read_fds;
    fd_set write_fds;
    FD_ZERO(&write_fds);

    //cycle of read-writes
    while (was_reading || was_writing)
    {
        was_reading = was_writing = 0;
        //if we must read something yet
        if (!fd_set_empty(max_fd+1, &total_read_fds))
        {
            select_err(max_fd+1, &read_fds, &write_fds, NULL, NULL); 
        }
        for (i = 0; i < a; ++i)
        {
            parallel_transfer(i, rbuffers, &read_fds, &write_fds,
                &was_reading, &was_writing, &total_read_fds);
        }

        //setting fd sets
        read_fds = total_read_fds;
        set_read_fds(rbuffers, &read_fds); //<---------
        set_write_fds(rbuffers, &write_fds);
    }

    //destruction of ring buffers
    for (i = 0; i < a; ++i)
    {
        ring_buf_dtor(&rbuffers[i]);
    }
}

int main (int argc, char** argv)
{
    errno = 0;

    //handling issue with wrong num of args
    if (argc != 3)
    {
        fprintf(stderr,
            "Wrong num of arguments. (There\'re %i args, must be 2)\n",
            argc - 1);
        exit(EXIT_FAILURE);
    }

    //getting a - number than is in argv[1]
    char* endptr = NULL;
    a = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "The argument is not a number.\n");
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE) 
    {
        fprintf(stderr, "The number is too big.\n");
        exit(EXIT_FAILURE);
    }

    //getting filename
    filename = argv[2];

    //creating pipes
    long i = 0;
    pipes = calloc(2*a, sizeof(int[2]));
    for (i = 0; i < 2*a; ++i)
    {
        pipes[i] = calloc(2, sizeof(int));
        pipe_err(pipes[i]);
    }

    //borning of children
    pid_t pid = 0;
    for (i = 0; i < a; ++i)
    {
        pid = fork_err();
        if (pid == 0)
        {
            child_work(i);
            break;
        }
    }

    //doing parent work
    if (pid > 0)
    {
        parent_work();
    }

    //freeing of all resources
    for(i = 0; i < 2*a; ++i)
    {
        free(pipes[i]);
    }
    free(pipes);
    return 0;
}
