#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char** argv)
{
    if (argc != 3)
    {
        return 0;
    }
    else
    {
        struct stat f1, f2;
        stat (argv[1], &f1);
        stat (argv[2], &f2);
        printf ("Inodes:%i,%i\n", f1.st_ino, f2.st_ino);
        printf ("Devics:%i,%i\n", f1.st_dev, f2.st_dev);
        if (f1.st_ino == f2.st_ino && f1.st_dev == f2.st_dev)
        {
            printf ("Files are the one file\n");
        }
        else
        {
            printf ("Files are two different files\n");
        }
    }
    return 0;
}
