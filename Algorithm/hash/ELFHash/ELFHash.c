#include "ELFHash.h"

unsigned int ELFHash(const char *str)
{
    unsigned int hash = 0;
    unsigned int x = 0;
    while (*str != '\0')
    {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000) != 0)
        {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }
    return hash;
}