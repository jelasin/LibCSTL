#include "JSHash.h"

unsigned int JSHash(const char *str)
{
    unsigned int hash = 1315423911;
    while (*str != '\0')
    {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }
    return hash;
}