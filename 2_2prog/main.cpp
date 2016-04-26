#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <sys/sysinfo.h>

typedef double T;

struct Data
{
    unsigned long num_of_cpus;
    unsigned long cur_cpu;
    unsigned long size;
    pthread_mutex_t* swap_mutex_p;
    pthread_mutex_t* cond2_mutex_p;
    pthread_cond_t* cv2_p;
    unsigned long* substracted_times;
    T* bas_string;
    T* sub_string;
    T** pointers;
};

void swap_strings(Data* const data_p, unsigned long a, unsigned long b)
{
    T** const pointers = data_p->pointers;
    unsigned long const size = data_p->size;

    double const sgn = pow(-1, std::max(a,b)-std::min(a,b));

    T* const bas_string = data_p->bas_string;

    for (unsigned long i = 0; i < size; ++i)
    {
        pointers[a][i] *= sgn;
    }

    memcpy(bas_string, pointers[a], size*sizeof(T));
    memcpy(pointers[a], pointers[b], size*sizeof(T));
    memcpy(pointers[b], bas_string, size*sizeof(T));
}

void swap_with_nonzero(Data* const data_p, unsigned long const str_num)
{
    T** const pointers = data_p->pointers;
    unsigned long const size = data_p->size;

    for(unsigned long i = str_num+1; i < size; ++i)
    {
        if(pointers[i][str_num] != 0)
        {
            swap_strings(data_p, str_num , i);
            return;
        }
    }
}

void substract_strings(Data* const data_p, unsigned long const i,
    unsigned long const str_num)
{
    T** const pointers = data_p->pointers;
    T* const bas_string = data_p->bas_string;
    unsigned long const size = data_p->size;

    T const mul = pointers[i][str_num] / bas_string[str_num];
    pointers[i][str_num] = 0.0;

    for(unsigned long j = str_num+1; j < size; ++j)
    {
        pointers[i][j] -= bas_string[j] * mul;
    }
}

void one_step(Data* const data_p, unsigned long const str_num)
{ 
    T** const pointers = data_p->pointers;
    T* const bas_string = data_p->bas_string;
    pthread_mutex_t* const cv2_mutex_p = data_p->cond2_mutex_p;
    pthread_cond_t* const cv2_p = data_p->cv2_p;
    unsigned long const cur_cpu = data_p->cur_cpu;
    unsigned long const num_of_cpus = data_p->num_of_cpus;
    unsigned long const size = data_p->size;

    unsigned long const amount_of_strs = size-str_num-1;
    for (unsigned long i = str_num+1+cur_cpu*amount_of_strs/num_of_cpus;
        i < str_num+1+(cur_cpu+1)*amount_of_strs/num_of_cpus; i++)
    {
        if (str_num)
        {
            pthread_mutex_lock(&cv2_mutex_p[str_num-1]);
            while (data_p->substracted_times[str_num-1] < size-str_num)
            {
                pthread_cond_wait(&cv2_p[str_num-1], &cv2_mutex_p[str_num-1]);
            }
            pthread_mutex_unlock(&cv2_mutex_p[str_num-1]);
        }

        //we swap string if cur diagonal elem is 0
        pthread_mutex_lock(data_p->swap_mutex_p);
        if (pointers[str_num][str_num] == 0)
        {
            assert(NULL);
            swap_with_nonzero(data_p, str_num);
        }
        pthread_mutex_unlock(data_p->swap_mutex_p);
        
        if (i==str_num+1+cur_cpu*amount_of_strs/num_of_cpus)
        {
            memcpy(bas_string, pointers[str_num], size*sizeof(T));
        }
        
        if (bas_string[str_num] != 0)
        {
            substract_strings(data_p, i, str_num);
        }
        else
        {
            assert(NULL);
        }

        pthread_mutex_lock(&cv2_mutex_p[str_num]);
        data_p->substracted_times[str_num]++;
        if (data_p->substracted_times[str_num] == size-str_num-1)
        {
            pthread_cond_broadcast(&cv2_p[str_num]);
        }
        pthread_mutex_unlock(&cv2_mutex_p[str_num]);
    }
}

void* get_triangle(void* data)
{
    Data* const data_p = (Data* const)data;
    unsigned long const size = data_p->size;

    //executing the one step of Gauss algo
    for (unsigned long str_num = 0; str_num < size; str_num++)
    {
        one_step(data_p, str_num);
    }
    return nullptr;
}

void print_matrix(T** matrix, unsigned long m_size)
{
    /*
    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < m_size; ++j)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
    */
}

int main(int argc, char** argv)
{
    //handling wrond num of args
    if (argc != 2)
    {
        std::cerr << "Usage: ./a.out NUM_OF_CPUS" << std::endl;
        exit(EXIT_FAILURE);
    }

    //creating the vector of threads
    unsigned long num_of_cpus = strtoul(argv[1], nullptr, 10);
    num_of_cpus = std::min(num_of_cpus, (long unsigned)get_nprocs());
    if (num_of_cpus < 1)
    {
        std::cerr << "Wrong num of cpus: " << num_of_cpus << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<pthread_t> threads_v(num_of_cpus);

    //initializing the matrix
    std::ifstream matrix_file;
    matrix_file.open("matrix2.txt");
    unsigned long m_size = 0;
    matrix_file >> m_size;
    T** matrix = nullptr;
    matrix = (T**)calloc(m_size, sizeof(T*));
    for (unsigned long i = 0; i < m_size; ++i)
    {
        matrix[i] = (T*)calloc(m_size, sizeof(T));
    }

    T** strings = nullptr;
    strings = (T**)calloc(2*num_of_cpus, sizeof(T*));
    for (unsigned long i = 0; i < 2*num_of_cpus; ++i)
    {
        strings[i] = (T*)calloc(m_size, sizeof(T));
    }

    //reading the matrix
    for (unsigned long i = 0; i < m_size; ++i)
    {
        for (unsigned long j = 0; j < m_size; ++j)
        {
            matrix_file >> matrix[i][j];
        }
    }
    print_matrix(matrix, m_size);

    //initializing the swap and cond mutexes (need for Gauss algorithm)
    pthread_mutex_t swap_mutex;
    pthread_mutex_init(&swap_mutex, nullptr);
    pthread_mutex_t* cond2_mutexes = new pthread_mutex_t[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        pthread_mutex_init(&cond2_mutexes[i], nullptr);
    }
    pthread_cond_t* cvs2 = new pthread_cond_t[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        pthread_cond_init(&cvs2[i], nullptr);
    }

    //initializing the "substracted_times" array
    unsigned long* substracted_times = new unsigned long[m_size];
    for (unsigned long i = 0; i < m_size; ++i)
    {
        substracted_times[i] = 0;
    }

    //initializing the "pointers" array
    T*** pointers = new T**[num_of_cpus];
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        pointers[i] = new T*[m_size];
        for (unsigned long j = 0; j < m_size; ++j)
        {
            pointers[i][j] = matrix[j]; 
        }
    }

    //calculating the args for threads
    Data* data = new Data[num_of_cpus];
    for (unsigned long i = 0; i < num_of_cpus; ++i)
    {
        data[i].num_of_cpus = num_of_cpus;
        data[i].swap_mutex_p = &swap_mutex;
        data[i].cond2_mutex_p = cond2_mutexes;
        data[i].cur_cpu = i;
        data[i].substracted_times = substracted_times;
        data[i].cv2_p = cvs2;
        data[i].size = m_size;
        data[i].sub_string = strings[i*2];
        data[i].bas_string = strings[i*2+1];
        data[i].pointers = pointers[i];
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    //executing the Gauss algo for creating the triangle matrix
    for (unsigned long j = 0; j < num_of_cpus; ++j)
    {
        pthread_create(&threads_v[j], nullptr, get_triangle, &data[j]);
    }
    for (unsigned long j = 0; j < num_of_cpus; ++j)
    {
        pthread_join(threads_v[j], nullptr);
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_secs = end-start;
    std::cout << "Time: " << elapsed_secs.count() << std::endl;

    //freeing the dynamic memory
    delete [] substracted_times;
    delete [] data;
    delete [] cvs2;
    delete [] cond2_mutexes;
    for (unsigned long j = 0; j < num_of_cpus; ++j)
    {
        delete [] pointers[j];
    }
    delete [] pointers;

    //multiplying the diag elems in order to get determinant
    double det = 1;
    for (unsigned long i = 0; i < m_size; ++i)
    {
        det *= matrix[i][i];
    }
    print_matrix(matrix, m_size);
    std::cout << det << std::endl;

    //freeing "strings"
    for (unsigned long i = 0; i < 2*num_of_cpus; ++i)
    {
        free(strings[i]);
    }
    free(strings);

    //freeing memory of matrix
    for (unsigned long i = 0; i < m_size; ++i)
    {
        free(matrix[i]);
    }
    free(matrix);


    return 0;
}
