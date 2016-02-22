#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const int BUF_SIZE = 4096;

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        printf ("Invalid number of arguments - %i - must be 1\n", argc - 1);
        exit(-1);
    }

    int file_descs[2] = {0, 0};
    int pipe_ret = pipe (file_descs);
    if (pipe_ret != 0)
    {
        printf ("Error in piping\n");
        exit(-1);
    }
    int pid = fork();
    if (pid == 0)
    {
        //Child
        close (file_descs[0]);
        int fd_to_send = open (argv[1],  O_RDONLY);
        char buffer[BUF_SIZE]; 
        int num_of_bytes = 1;
        while (num_of_bytes > 0)
        {
            num_of_bytes = read (fd_to_send, buffer, BUF_SIZE);
            write (file_descs[1], buffer, num_of_bytes);
        }
        close (file_descs[1]);
    }
    else if (pid > 0)
    {
        //Parent
        close (file_descs[1]);
        char buffer[BUF_SIZE]; 
        int num_of_bytes = 1;
        while (num_of_bytes > 0)
        {
            num_of_bytes = read (file_descs[0], buffer, BUF_SIZE);
            write (STDOUT_FILENO, buffer, num_of_bytes);
        }
        close (file_descs[0]);
    }
    else
    {
        printf ("Error in forking\n");
        exit(-1);
    }
    return 0;
}
