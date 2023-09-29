#include <assert.h>
#include <stdio.h>

#include "my_hash.h"

const size_t DENOMINATOR = 3; // prostoye

Hash_t HashMod(const void * val, int size) {

    assert (val);

    Hash_t hash = 0;

    int char_num = size % sizeof(int);

    for (int i = 0; i < size / sizeof(int); i++) {
        hash += *(int * ) val % DENOMINATOR;
        hash %= DENOMINATOR;
        val = (int *) val + 1;
    }

    for (int i = 0; i < char_num; i++) {
        hash += *(char *) val % DENOMINATOR;
        hash %= DENOMINATOR;
        val = (char *) val + 1;
    }

    return hash;
}
