#include "../syscall_err.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

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

    for(unsigned long i = 0; i < m_p->size1(); ++i)
    {
        T tmp = (*m_p)(a, i);
        (*m_p)(a, i) = (*m_p)(b, i);
        (*m_p)(b, i) = tmp;
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

    if ((*m_p)(str_num, str_num) == 0)
    {
        pthread_mutex_lock(data_p->swap_mutex_p);
        swap_with_nonzero(data_p);
        pthread_mutex_unlock(data_p->swap_mutex_p);
    }
    if ((*m_p)(str_num, str_num) == 0)
    {
        return nullptr;
    }
    for (unsigned long i = str_num+1+cur_cpu; i < m_p->size1(); i+=num_of_cpus)
    {
        T mul = (*m_p)(i, str_num) / (*m_p)(str_num, str_num);
        for(unsigned long j = str_num; j < m_p->size1(); ++j)
        {
            (*m_p)(i, j) -= (*m_p)(str_num, j) * mul;
        }
    }
    return nullptr;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cerr << "Usage: ./a.out NUM_OF_CPUS MATRIX_FILE" << endl;
        exit(EXIT_FAILURE);
    }

    unsigned long num_of_cpus = strtoul_err(argv[1], nullptr, 10);
    std::vector<pthread_t> threads_v(num_of_cpus);

    ifstream file;
    file.open(argv[2]);
    unsigned long m_size = 0;
    file >> m_size;
    matrix<T> matrix(m_size, m_size);

    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < m_size; ++j)
        {
            file >> matrix(i,j);
        }
    }
    cout << matrix << endl;

    Data data;
    data.matrix_p = &matrix;
    data.num_of_cpus = num_of_cpus;
    pthread_mutex_t swap_mutex;
    data.swap_mutex_p = &swap_mutex;
    pthread_mutex_init(data.swap_mutex_p, nullptr);
    for (unsigned long i = 0; i < m_size; ++i)
    {
        data.str_num = i;

        for (unsigned long j = 0; j < num_of_cpus; ++j)
        {
            data.cur_cpu = j;
            pthread_create_err(&threads_v[j], nullptr, get_triangle, &data);
        }
        for (unsigned long j = 0; j < num_of_cpus; ++j)
        {
            pthread_join_err(threads_v[j], nullptr);
        }
    }
    pthread_mutex_destroy(data.swap_mutex_p);
    cout << matrix << endl;

    return 0;
}
