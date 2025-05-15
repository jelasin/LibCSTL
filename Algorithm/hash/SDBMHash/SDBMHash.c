#include "SDBMHash.h"

unsigned int SDBMHash(const char *str)
{
    unsigned int hash = 0;
    while (*str != '\0')
    {
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
