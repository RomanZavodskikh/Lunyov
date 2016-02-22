#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define FIFO_NAME "fifo.klichko"
#define CHSEN_NAME "fifo.klichko.say"
#define CHREC_NAME "fifo.obama.say"

const int SENDER = 1;
const int RECEIVER = 2;
const unsigned BUF_SIZE = 4096; 

struct msg_type
{
    long type;
    long value;
};

long calc_buffer_max()
{
    char str[10];
    sprintf(str, "%d", getpid());
    mkfifo (str, 0666);
    int fd1 = open (str, O_RDONLY|O_NONBLOCK); 
    int fd = open (str, O_WRONLY|O_NONBLOCK);
    long rtr_val = 0;
    while (write(fd, "0", 1) != -1)
    {
        ++rtr_val;
    }
    close(fd);
    close(fd1);
    unlink(str);
    return rtr_val;
}

void sender_work (char** argv)
{
    struct stat stat_f_to_send;
    if (stat(argv[1], &stat_f_to_send) == -1)
    {
        fprintf (stderr, "Stat error (in sender).\n");
        exit(-1);
    }
    if ((stat_f_to_send.st_mode & S_IFMT) == S_IFIFO)
    {
        fprintf (stderr, "Cannot send fifo through fifo (in sender).\n");
        exit(-1);
    }

    long BUFFER_MAX = calc_buffer_max();

    mkfifo (CHSEN_NAME, 0666);
    int fd_fifo2 = open (CHSEN_NAME, O_RDONLY|O_NONBLOCK);
    int fd_fifo = open (CHSEN_NAME, O_WRONLY|O_NONBLOCK);
    char* max_buf = malloc(BUFFER_MAX * sizeof(*max_buf));
    memset(max_buf, '0', BUFFER_MAX);
    int write_rt = write (fd_fifo, max_buf, BUFFER_MAX);
    if (write_rt == -1)
    {
        fprintf (stderr, "There\'s another sender.\n");
        exit (-1);
    }


    char buffer[BUF_SIZE];
    mkfifo (FIFO_NAME, 0666);
    fd_fifo = open (FIFO_NAME, O_WRONLY);
    int fd_to_send = open (argv[1], O_RDONLY);
    int num_of_bytes = 1;
    while (num_of_bytes > 0)
    {
        num_of_bytes = read (fd_to_send, buffer, BUF_SIZE);
        write (fd_fifo, buffer, num_of_bytes);
    }
    close (fd_fifo);
    close (fd_fifo2);
    unlink(FIFO_NAME);
    close (fd_to_send);
    unlink(CHSEN_NAME);
}

void receiver_work(char** argv)
{
    long BUFFER_MAX = calc_buffer_max();

    mkfifo (FIFO_NAME, 0666);
    mkfifo (CHREC_NAME, 0666);
    int fd_fifo2 = open (CHREC_NAME, O_RDONLY|O_NONBLOCK);
    int fd_fifo = open (CHREC_NAME, O_WRONLY|O_NONBLOCK);
    char* max_buf = malloc(BUFFER_MAX * sizeof(*max_buf));
    memset(max_buf, '0', BUFFER_MAX);
    int write_rt = write (fd_fifo, max_buf, BUFFER_MAX);
    if (write_rt == -1)
    {
        fprintf (stderr, "There\'s another receiver.\n");
        exit (-1);
    }

    char buffer[BUF_SIZE];
    fd_fifo = open (FIFO_NAME, O_RDONLY);
    int num_of_bytes = 1;
    while (num_of_bytes > 0)
    {
        num_of_bytes = read (fd_fifo, buffer, BUF_SIZE);
        write (STDOUT_FILENO, buffer, num_of_bytes);
    }
    close (fd_fifo);
    close (fd_fifo2);
    unlink(CHREC_NAME);
}

int main (int argc, char** argv)
{
    if (argc == 1)
    {
        receiver_work(argv);
    }
    else if (argc == 2)
    {
        sender_work(argv);
    }
    else
    {
        fprintf (stderr, "There must be 0 or 1 arguments, but there\'re %i\n", argc - 1);
        exit(-1);
    }
    return 0;
}
