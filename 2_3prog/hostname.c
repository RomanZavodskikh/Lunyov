#include <unistd.h>

#include <stdio.h>

int main()
{
    char str[100];
    gethostname(str, 100);
    printf("%s\n", str);
    return 0;
}
