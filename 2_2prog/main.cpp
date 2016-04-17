#include "../syscall_err.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/sysinfo.h>

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
    pthread_mutex_t* cond_mutex_p;
    pthread_mutex_t* cond2_mutex_p;
    pthread_cond_t* cv_p;
    pthread_cond_t* cv2_p;
    unsigned long* substracted_times;
    unsigned long* has_zeros;
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

void one_step(void* data)
{ 
    Data* const data_p = (Data*) data;
    matrix<T>* const m_p = data_p->matrix_p;
    unsigned long const str_num = data_p->str_num;
    pthread_mutex_t* const cv_mutex_p = data_p->cond_mutex_p;
    pthread_mutex_t* const cv2_mutex_p = data_p->cond2_mutex_p;
    pthread_cond_t* const cv_p = data_p->cv_p;
    pthread_cond_t* const cv2_p = data_p->cv2_p;

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
        return;
    }

    for (unsigned long i = str_num+1; i < m_p->size1(); i++)
    {
        pthread_mutex_lock(cv_mutex_p);
        while (data_p->substracted_times[str_num] != str_num)
        {
            pthread_cond_wait(&cv_p[str_num], cv_mutex_p);
        }
        pthread_mutex_unlock(cv_mutex_p);

        pthread_mutex_lock(cv2_mutex_p);
        while (data_p->has_zeros[i] < str_num)
        {
            pthread_cond_wait(&cv2_p[i], cv2_mutex_p);
        }
        pthread_mutex_unlock(cv2_mutex_p);

        T mul = (*m_p)(i, str_num) / (*m_p)(str_num, str_num);
        (*m_p)(i, str_num) = 0.0;

        for(unsigned long j = str_num+1; j < m_p->size1(); ++j)
        {
            (*m_p)(i, j) -= (*m_p)(str_num, j) * mul;
        }

        pthread_mutex_lock(cv2_mutex_p);
        data_p->has_zeros[i]++;
        pthread_cond_broadcast(&cv2_p[i]);
        pthread_mutex_unlock(cv2_mutex_p);

        pthread_mutex_lock(cv_mutex_p);
        data_p->substracted_times[i]++;
        if (data_p->substracted_times[i] == i)
        {
            pthread_cond_broadcast(&cv_p[i]);
        }
        pthread_mutex_unlock(cv_mutex_p);
    }
}

void* get_triangle(void* data)
{
    Data* const data_p = (Data*) data;
    matrix<T>* const m_p = data_p->matrix_p;
    unsigned long const num_of_cpus = data_p->num_of_cpus;
    unsigned long const cur_cpu = data_p->cur_cpu;

    //executing the one step of Gauss algo
    for (data_p->str_num = cur_cpu; data_p->str_num < m_p->size1(); 
            data_p->str_num+=num_of_cpus)
    {
        one_step(data_p);
    }
    return nullptr;
}

template<class T>
void print_matrix(matrix<T>* matrix)
{
    /*
    for (unsigned long i = 0; i < matrix->size1(); ++i)
    {
        for (unsigned long j = 0; j < matrix->size2(); ++j)
        {
            cout << (*matrix)(i,j) << " ";
        }
        cout << endl;
    }
    */
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
    num_of_cpus = min(num_of_cpus, (long unsigned)get_nprocs());
    if (num_of_cpus < 1)
    {
        cerr << "Wrong num of cpus: " << num_of_cpus << endl;
        exit(EXIT_FAILURE);
    }
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
    print_matrix(&matrix);

    //initializing the swap and cond mutexes (need for Gauss algorithm)
    pthread_mutex_t swap_mutex;
    pthread_mutex_init(&swap_mutex, nullptr);
    pthread_mutex_t cond_mutex;
    pthread_mutex_init(&cond_mutex, nullptr);
    pthread_mutex_t cond2_mutex;
    pthread_mutex_init(&cond2_mutex, nullptr);
    pthread_cond_t* cvs = new pthread_cond_t[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        pthread_cond_init(&cvs[i], nullptr);
    }
    pthread_cond_t* cvs2 = new pthread_cond_t[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        pthread_cond_init(&cvs2[i], nullptr);
    }

    //initializing the "substracted_strings" array
    unsigned long* substracted_times = new unsigned long[m_size];
    unsigned long* has_zeros = new unsigned long[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        substracted_times[i] = 0;
        has_zeros[i] = 0;
    }

    //calculating the args for threads
    Data* data = new Data[num_of_cpus];
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        data[i].matrix_p = &matrix;
        data[i].num_of_cpus = num_of_cpus;
        data[i].swap_mutex_p = &swap_mutex;
        data[i].cond_mutex_p = &cond_mutex;
        data[i].cond2_mutex_p = &cond2_mutex;
        data[i].cv_p = cvs;
        data[i].str_num = 0;
        data[i].substracted_times =  substracted_times;
        data[i].cur_cpu = i;
        data[i].has_zeros = has_zeros;
        data[i].cv2_p = cvs2;
    }

    //executing the Gauss algo for creating the triangle matrix
    for (unsigned long j = 0; j < num_of_cpus; ++j)
    {
        pthread_create_err(&threads_v[j], nullptr, get_triangle, &data[j]);
    }
    for (unsigned long j = 0; j < num_of_cpus; ++j)
    {
        pthread_join_err(threads_v[j], nullptr);
    }

    //multiplying the diag elems in order to get determinant
    double det = 1;
    for (unsigned long i = 0; i < m_size; ++i)
    {
        det *= matrix(i,i);
    }
    print_matrix(&matrix);
    cout << det << endl;

    return 0;
}
