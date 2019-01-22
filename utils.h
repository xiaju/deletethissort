// Author: Nat Tuck
// CS3650 starter code

#ifndef UTILS_H
#define UTILS_H

#include "barrier.h"

typedef struct sort_job {
    int pnum;
    floats* input;
    const char* output;
    int P;
    floats* samps;
    long* sizes;
    barrier* bb;
} sort_job;

void seed_rng();
void check_rv(int rv);
int comp(const void* a, const void* b);

#endif

