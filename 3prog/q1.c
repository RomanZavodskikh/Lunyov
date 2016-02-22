#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main ()
{
    char buf[10];
    mkfifo("fifo", 666);
    int fd = open("fifo", O_RDWR);
    write(fd, "aaa", 3);
    close(fd);
    fd = open("fifo", O_RDONLY|O_NONBLOCK);
    write(1, buf, read(fd, buf,3));
    unlink("fifo");
    return 0;
}
