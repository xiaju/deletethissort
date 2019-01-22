// Author: Nat Tuck
// CS3650 starter code

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include "barrier.h"

barrier*
make_barrier(int nn)
{
    barrier* bb = malloc(sizeof(barrier));
    assert(bb != 0);

    bb->count = nn;  // TODO: These can't be right.
    bb->seen  = 0;

    pthread_mutex_init(&bb->m, 0);
    pthread_cond_init(&bb->c, 0);
    return bb;
}

void
barrier_wait(barrier* bb)
{
    pthread_mutex_lock(&bb->m);
    bb->seen++;
    while (bb->seen < bb->count) {
        // TODO: Stop waiting.
        // TODO: Don't sleep here.
        pthread_cond_wait(&bb->c, &bb->m);
    }
    pthread_cond_broadcast(&bb->c);
    pthread_mutex_unlock(&bb->m);
}

void
free_barrier(barrier* bb)
{
    free(bb);
}

