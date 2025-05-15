#ifndef __SIMPLE_HASH_H__
#define __SIMPLE_HASH_H__

void set_tableSize(unsigned int size);

unsigned int DivisionHash(const char *str);

unsigned int MultiplicationHash(const char *str);

#endif // __SIMPLE_HASH_H__