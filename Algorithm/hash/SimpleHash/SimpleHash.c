#include "SimpleHash.h"

static unsigned int tableSize = 10;

void set_tableSize(unsigned int size)
{
    tableSize = size;
}

unsigned int DivisionHash(const char *str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash += *str++;
    }
    return hash % tableSize;
}

unsigned int MultiplicationHash(const char *str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash += *str++;
    }
    return (unsigned int)(tableSize * (hash * 0.6180339887 - (unsigned int)(hash * 0.6180339887)));
}