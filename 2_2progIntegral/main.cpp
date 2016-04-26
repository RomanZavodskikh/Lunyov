#include <iostream>

#include <cmath>
#include <sys/sysinfo.h>
#include <pthread.h>

#include "../syscall_err.h"

using namespace std;

typedef double T;
typedef T(*F_P)(T);

T const STEP = 0.000001;
T const LEFT = 0.0;
T const RIGHT = 67.0;

struct Data
{
    T left;
    T right;
    T* sum_p;
    T (*foo)(T);
    unsigned long num_of_cpus;
    unsigned long cur_cpu;
};

T func (T x)
{
    return sin(x);
}

void init_args(Data* const args, unsigned long const num_of_cpus)
{
    T const delta = RIGHT - LEFT;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        args[i].left = LEFT + i*delta/num_of_cpus;
        args[i].right = LEFT + (i+1)*delta/num_of_cpus;
        args[i].sum_p = new T;
        args[i].foo = func;
        args[i].num_of_cpus = num_of_cpus;
        args[i].cur_cpu = i;
    }
}

void deinit_args(Data* const args, unsigned long const num_of_cpus)
{
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        delete args[i].sum_p;
    }
}

void* add_integral(void* args)
{
    T const left = ((Data*)args)->left;
    T const right = ((Data*)args)->right;
    T* const sum_p = ((Data*)args)->sum_p;
    T (*foo)(T) = ((Data*)args)->foo;

    T sum = 0;

    for (T x = left; x < right; x+=STEP)
    {
        sum += sin(x)*STEP;
    }

    *sum_p = sum;

    return nullptr;
}

T sum_sums(const Data* const args, unsigned long const num_of_cpus)
{
    T rtr_val = 0;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        rtr_val += *(args[i].sum_p);
    }
    return rtr_val;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Usage: ./a.out NUM_OF_CPUS" << endl;
        exit(EXIT_FAILURE);
    }
    unsigned long num_of_cpus = strtoul_err(argv[1], nullptr, 10);
    num_of_cpus = min(num_of_cpus, (long unsigned)get_nprocs());

    pthread_t* threads = new pthread_t[num_of_cpus];
    Data* args = new Data[num_of_cpus];

    init_args(args, num_of_cpus);

    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_create_err(&threads[i], nullptr, add_integral, &args[i]); 
    }
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_join_err(threads[i], nullptr);
    }

    T const sum = sum_sums(args, num_of_cpus);
    cout << sum << endl;

    deinit_args(args, num_of_cpus);

    delete [] args;
    delete [] threads;

    return 0;
}
