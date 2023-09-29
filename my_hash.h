#ifndef MY_HASH_H
#define MY_HASH_H

#include <stdio.h>

typedef size_t Hash_t;

Hash_t HashMod(const void * val, int size);

#endif // MY_HASH_H
