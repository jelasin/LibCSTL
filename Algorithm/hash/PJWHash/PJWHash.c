#include "PJWHash.h"

unsigned int PJWHash(const char *str)
{
    unsigned int hash = 0;
    unsigned int highBits = 0;
    while (*str != '\0')
    {
        hash = (hash << 4) + (*str++);
        if ((highBits = hash & 0xF0000000) != 0)
        {
            hash ^= (highBits >> 24);
        }
        hash &= ~highBits;
    }
    return hash;
}