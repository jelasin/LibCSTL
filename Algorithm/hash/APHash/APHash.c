#include "APHash.h"

unsigned int APHash(const char *str)
{
    unsigned int hash = 0;
    for (unsigned int i = 0; str[i] != '\0'; i++)
    {
        if ((i & 1) == 0) // Odd index
        {
            hash ^= ((hash << 7) ^ str[i] ^ (hash >> 3));
        }
        else // Even index
        {
            hash ^= (~((hash << 11) + str[i] + (hash >> 5)));
        }
    }
    return hash;
}