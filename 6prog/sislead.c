#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "../syscall_err.h"

int main(void)
{
    pid_t pid = fork_err();
    if (pid != 0)
    {
        exit(0);
    }
    int fd = open_err("/dev/pts/2", 0620);
    setsid_err();
    ioctl_err(fd, TIOCSCTTY, 1);
    int i = 0;
    for (i = 0; i < 10; ++i)
    {
        fork_err();
    }
    pause();
    return 0;
}
