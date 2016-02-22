#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    if (argc==1)
    {
        printf("There\'re too few arguments. Nothing to execute\n");
        exit(-1);
    }
    execvp (argv[1], &argv[1]);
    printf ("Error in executing %s!\n", argv[1]);
    return 0;
}
