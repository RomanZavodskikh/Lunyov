#ifndef BAD_FUNCS_H
#define BAD_FUNCS_H

#include <stdlib.h>

#define LUCK 200

void* calloc_bad(size_t nmemb, size_t size)
{
    if (rand()%LUCK==0)
    {
        return NULL;
    }
    else
    {
        return calloc(nmemb, size);
    }
}

void* memcpy_bad(void* dest, const void* src, size_t n)
{
    return NULL;
}

#endif //BAD_FUNCS_H
