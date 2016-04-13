#include "../syscall_err.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

const unsigned long M_SIZE = 9;

using namespace std;
using namespace boost::numeric::ublas;
 
typedef double T;

struct Data
{
    matrix<T>* matrix_p;
    unsigned long str_num;
    unsigned long num_of_cpus;
    unsigned long cur_cpu;
    pthread_mutex_t* swap_mutex_p;
};

void swap_strings(void* data, unsigned long a, unsigned long b)
{
    Data* const data_p = (Data*) data;
    matrix<T>* const m_p = data_p->matrix_p;

    double const sgn = pow(-1, max(a,b)-min(a,b));

    for(unsigned long i = 0; i < m_p->size1(); ++i)
    {
        T tmp = (*m_p)(a, i);
        (*m_p)(a, i) = (*m_p)(b, i);
        (*m_p)(b, i) = sgn*tmp;
    }
}

void swap_with_nonzero(void* data)
{
    Data* const data_p = (Data*) data;
    matrix<T>* const m_p = data_p->matrix_p;
    unsigned long const str_num = data_p->str_num;

    for(unsigned long i = str_num+1; i < m_p->size1(); ++i)
    {
        if((*m_p)(i, str_num) != 0)
        {
            swap_strings(data, str_num , i);
            return;
        }
    }
}

void* get_triangle(void* data)
{
    Data* const data_p = (Data*) data;
    matrix<T>* const m_p = data_p->matrix_p;
    unsigned long const str_num = data_p->str_num;
    unsigned long const num_of_cpus = data_p->num_of_cpus; 
    unsigned long const cur_cpu = data_p->cur_cpu;

    pthread_mutex_lock(data_p->swap_mutex_p);
    if ((*m_p)(str_num, str_num) == 0)
    {
        swap_with_nonzero(data_p);
    }
    pthread_mutex_unlock(data_p->swap_mutex_p);
    if ((*m_p)(str_num, str_num) == 0)
    {
        return nullptr;
    }
    for (unsigned long i = str_num+1+cur_cpu; i < m_p->size1(); i+=num_of_cpus)
    {
        T mul = (*m_p)(i, str_num) / (*m_p)(str_num, str_num);
        (*m_p)(i, str_num) = 0.0;
        for(unsigned long j = str_num+1; j < m_p->size1(); ++j)
        {
            (*m_p)(i, j) -= (*m_p)(str_num, j) * mul;
        }
    }
    return nullptr;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Usage: ./a.out NUM_OF_CPUS" << endl;
        exit(EXIT_FAILURE);
    }

    unsigned long num_of_cpus = strtoul_err(argv[1], nullptr, 10);
    num_of_cpus = min(num_of_cpus, 
        (unsigned long)std::thread::hardware_concurrency());
    std::vector<pthread_t> threads_v(num_of_cpus);

    unsigned long m_size = M_SIZE;
    matrix<T> matrix(m_size, m_size);

    /*
    const double elems[M_SIZE][M_SIZE]= {
    {  4,  1,  0,  1,  4,  9,  8,  9,  1},
    { 39, 13,  1,  4,  4,  9,  1,  1,  2},
    {  8,  2,  1,  4,  3, 99, 42, 43,  0},
    { 79, 64,  9,  2,  0, 24,  1, 83, 12},
    {  8,  4,  5, 43,  1,  1,  1, 21,  3},
    { 48,  9,  0,  0,  0,  0,  0, 42,  3},
    {  8,  9,  0, 14,  1,  3,  0,  8,  3},
    {  1, 23, 42, 52, 42, 42,  3,  4,  1},
    {  1,  1,  1,  3, 52,  1,  1,  2,  0} };
    */

    const double elems[M_SIZE][M_SIZE]= {
    {  0,  1,  0,  1,  4,  9,  8,  9,  1},
    { 39,  0,  1,  4,  4,  9,  1,  1,  2},
    {  8,  2,  0,  4,  3, 99, 42, 43,  0},
    { 79, 64,  9,  2,  0, 24,  1, 83, 12},
    {  8,  4,  5, 43,  1,  1,  1, 21,  3},
    { 48,  9,  0,  0,  0,  0,  0, 42,  3},
    {  8,  9,  0, 14,  1,  3,  0,  8,  3},
    {  1, 23, 42, 52, 42, 42,  3,  4,  1},
    {  1,  1,  1,  3, 52,  1,  1,  2,  0} };

    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < m_size; ++j)
        {
            matrix(i,j) = elems[i][j];
        }
    }
    cout << matrix << endl;

    Data* data = new Data[num_of_cpus];
    pthread_mutex_t swap_mutex;
    pthread_mutex_init(&swap_mutex, nullptr);
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        data[i].matrix_p = &matrix;
        data[i].num_of_cpus = num_of_cpus;
        data[i].swap_mutex_p = &swap_mutex;
    }
    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < num_of_cpus; ++j)
        {
            data[j].str_num = i;
            data[j].cur_cpu = j;
            pthread_create_err(&threads_v[j], nullptr, get_triangle, &data[j]);
        }
        for (unsigned long j = 0; j < num_of_cpus; ++j)
        {
            pthread_join_err(threads_v[j], nullptr);
        }
    }
    pthread_mutex_destroy(&swap_mutex);
    cout << matrix << endl;

    double det = 1;
    for (unsigned long i = 0; i < m_size; ++i)
    {
        det *= matrix(i,i);
    }
    cout << fixed << det << endl;

    return 0;
}
