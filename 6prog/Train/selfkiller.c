#include <stdio.h>
#include <signal.h>

int main()
{
    raise(SIGKILL);
    printf("This string MUST BE NEVER BE SEEN!\n");
    return 0;
}
