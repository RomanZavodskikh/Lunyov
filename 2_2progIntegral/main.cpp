#include <iostream>

#include <cmath>
#include <sys/sysinfo.h>
#include <pthread.h>

using namespace std;

typedef double T;

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

void init_args(Data* args, unsigned long const num_of_cpus, T* const sum_p)
{
    T const delta = RIGHT - LEFT;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        args[i].left = LEFT + i*delta/num_of_cpus;
        args[i].right = LEFT + (i+1)*delta/num_of_cpus;
        args[i].sum_p = &sum_p[i];
        args[i].foo = func;
        args[i].num_of_cpus = num_of_cpus;
        args[i].cur_cpu = i;
    }
}

void* add_integral(void* args)
{
    T const left = ((Data*)args)->left;
    T const right = ((Data*)args)->right;
    T* const sum_p = ((Data*)args)->sum_p;
    T (*foo)(T) = ((Data*)args)->foo;

    for (T x = left; x < right; x+=STEP)
    {
        *sum_p += foo(x)*STEP;
    }

    return nullptr;
}

T sum_sums(const T* const sum_p, unsigned long const num_of_cpus)
{
    T rtr_val = 0;
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        rtr_val += sum_p[i];
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
    unsigned long num_of_cpus = strtoul(argv[1], nullptr, 10);
    num_of_cpus = min(num_of_cpus, (long unsigned)get_nprocs());

    T* sum_p = new T[num_of_cpus];
    pthread_t* threads = new pthread_t[num_of_cpus];
    Data* args = new Data[num_of_cpus];

    init_args(args, num_of_cpus, sum_p);

    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_create(&threads[i], nullptr, add_integral, &args[i]); 
    }
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    cout << sum_sums(sum_p, num_of_cpus) << endl;

    delete [] args;
    delete [] threads;
    delete sum_p;

    return 0;
}
