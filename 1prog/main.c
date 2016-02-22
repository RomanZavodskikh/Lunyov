#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

int main (int argc, char** argv)
{
    errno = 0;
    if (argc != 2)
    {
        printf("Wrong num of arguments. (There\'re %i args, must be 1)\n", argc - 1);
        assert(NULL);
    }

    int i = 0;
    char* endptr = NULL;
    long a = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        printf("The argument is not a number.\n");
        assert(NULL);
    }
    if (errno == ERANGE) 
    {
        printf("The number is too big.\n");
        assert(NULL);
    }
    if (a >= 1)
    {
        for (i = 1; i <= a; ++i)
        {
            printf ("%d ", i);
        }
    }
    else
    {
        for (i = 0; i >= a; --i)
        {
            printf ("%d ", i);
        }
    }
    printf("\n");
    return 0;
}
