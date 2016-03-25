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
};

void* normalize_string(void* data)
{
    Data* const data_p = (Data*) data;
    if ((*data_p->matrix_p)(data_p->str_num, data_p->str_num) == 0)
    {
        return nullptr;
    }
    T divisor = (*data_p->matrix_p)(data_p->str_num, data_p->str_num);
    for (unsigned long i = 0; i < data_p->matrix_p->size1(); ++i)
    {
        (*data_p->matrix_p)(data_p->str_num, i) /= divisor;
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

    Data* data = new Data[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        data[i].matrix_p = &matrix;
        data[i].str_num = i;
    }
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_create_err(&threads_v[i], nullptr, normalize_string, &data[i]);
    }
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pthread_join_err(threads_v[i], nullptr);
    }
    cout << matrix << endl;

    return 0;
}
