#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include "float.h"
#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the floats
    qsort(xs->data, xs->size, sizeof(float), comp);
}

// sample code referenced from previous assignment
floats*
sample(floats* input, int P)
{
    // TODO: Sample some floats.
    floats* samps = make_floats(0);
    float pickMed[3 * (P - 1)];

    for (int i = 0; i < 3 * (P - 1); i++) {
        long ind = rand() % input->size;
        pickMed[i] = input->data[ind];
    }

    qsort(pickMed, 3 * (P - 1), sizeof(float), comp);

    floats_push(samps, FLT_MIN);

    for (int i = 0; i < P - 1; i++) {
        floats_push(samps, pickMed[1 + 3 * i]);
    }

    floats_push(samps, INFINITY);

    return samps;
}
// partition code referenced from previous assignment
void
sort_worker(int pnum, floats* input, const char* output, int P, floats* samps, long* sizes, barrier* bb)
{
    floats* xs = make_floats(0);

    // TODO: Build a local array
    for (long j = 0; j < input->size; j++) {
        if (input->data[j] > samps->data[pnum] - .00001 && input->data[j] < samps->data[pnum + 1]) {
            floats_push(xs, input->data[j]);
        }
    }
    sizes[pnum] = xs->size;
    
    printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);

    // TODO: Sort it

    qsort_floats(xs);

    // TODO: open(2) the output file
    int ofd = open(output, O_WRONLY);

    // TODO: lseek(2) to the right spot
    barrier_wait(bb);
    
    int offset = 0;
    for (long l = 0; l < pnum; l++) {
        offset += sizes[l];
    }

    lseek(ofd, sizeof(long) + offset * sizeof(float), SEEK_SET);

    // TODO: Write your local array with write(2)
    write(ofd, xs->data, xs->size * sizeof(float));
   
    close(ofd); 
    free_floats(xs);
}

void*
sort_worker_help(void* arg)
{
    sort_job job = *((sort_job*) arg);
    sort_worker(job.pnum, job.input, job.output, job.P, job.samps, job.sizes, job.bb);
    return 0;
}

void
run_sort_workers(floats* input, const char* output, int P, floats* samps, long* sizes, barrier* bb)
{
    pthread_t threads[P];
    sort_job s[P];

    // TODO: Spawn P threads running sort_worker
    // TODO: wait for all P threads to complete

    for (int i = 0; i < P; i++) {
        s[i].pnum = i;
        s[i].input = input;
        s[i].output = output;
        s[i].P = P;
        s[i].samps = samps;
        s[i].sizes = sizes;
        s[i].bb = bb;
        pthread_create(&threads[i], 0, sort_worker_help, &s[i]);
    }

    for (int i = 0; i < P; i++) {
        int rv = pthread_join(threads[i], 0);
        assert(rv == 0);
    }
}

void
sample_sort(floats* input, const char* output, int P, long* sizes, barrier* bb)
{
    floats* samps = sample(input, P);
    run_sort_workers(input, output, P, samps, sizes, bb);
    free_floats(samps);
}

// threading code and use of open syscalls referenced from Nat Tuck's lecture notes
int
main(int argc, char* argv[])
{
    alarm(120);

    if (argc != 4) {
        printf("Usage:\n");
        printf("\t%s P input.dat output.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* iname = argv[2];
    const char* oname = argv[3];

    // TODO: remove this print
    //printf("Sort from %s to %s.\n", iname, oname);

    seed_rng();

    // TODO: Open the input file and read the data into the input array.
    int ifd = open(iname, O_RDONLY); 

    long size;
    read(ifd, &size, sizeof(long));

    floats* input = make_floats(size);

    read(ifd, input->data, size * sizeof(float));
    // TODO: Create the output file, of the same size, with ftruncate(2)
    // TODO: Write the size to the output file.

    int ofd = open(oname, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    check_rv(ofd);

    ftruncate(ofd, sizeof(long) + size * sizeof(float));
    write(ofd, &size, sizeof(long));


    int rv = close(ofd);
    check_rv(rv);

    barrier* bb = make_barrier(P);

    long* sizes = malloc(P * sizeof(long));
    sample_sort(input, oname, P, sizes, bb);

    free(sizes);

    free_barrier(bb);
    free_floats(input);

    return 0;
}

