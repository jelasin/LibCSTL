#include "RSHash.h"

unsigned int RSHash(const char *str)
{
    unsigned int hash = 0;
    unsigned int tmp;
    while (*str != '\0')
    {
        hash = (hash << 5) ^ (hash >> 27) ^ (*str++);
    }
    return hash;
}
