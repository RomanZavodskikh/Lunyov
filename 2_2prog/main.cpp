#include "../syscall_err.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>

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

    //we swap string if cur diagonal elem is 0
    pthread_mutex_lock(data_p->swap_mutex_p);
    if ((*m_p)(str_num, str_num) == 0)
    {
        swap_with_nonzero(data_p);
    }
    pthread_mutex_unlock(data_p->swap_mutex_p);

    //go away if all elems lower is 0 - determinant is known. It's 0.
    if ((*m_p)(str_num, str_num) == 0)
    {
        return nullptr;
    }

    //executing the one step of Gauss algo
    //str_num is the lowest string that is computed correctly already
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
    //handling wrond num of args
    if (argc != 2)
    {
        cerr << "Usage: ./a.out NUM_OF_CPUS" << endl;
        exit(EXIT_FAILURE);
    }

    //creating the vector of threads
    unsigned long num_of_cpus = strtoul_err(argv[1], nullptr, 10);
    num_of_cpus = min(num_of_cpus, 
        (unsigned long)std::thread::hardware_concurrency());
    std::vector<pthread_t> threads_v(num_of_cpus);

    //initializing the matrix
    std::ifstream matrix_file;
    matrix_file.open("matrix2.txt");
    unsigned long m_size = 0;
    matrix_file >> m_size;
    matrix<T> matrix(m_size, m_size);

    //reading the matrix
    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < m_size; ++j)
        {
            matrix_file >> matrix(i,j);
        }
    }

    //initializing the swap mutex (need for Gauss algorithm)
    pthread_mutex_t swap_mutex;
    pthread_mutex_init(&swap_mutex, nullptr);

    //calculating the args for threads
    Data* data = new Data[num_of_cpus];
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        data[i].matrix_p = &matrix;
        data[i].num_of_cpus = num_of_cpus;
        data[i].swap_mutex_p = &swap_mutex;
    }

    //executing the Gauss algo for creating the triangle matrix
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

    //multiplying the diag elems in order to get determinant
    double det = 1;
    for (unsigned long i = 0; i < m_size; ++i)
    {
        det *= matrix(i,i);
    }
    cout << det << endl;

    return 0;
}
