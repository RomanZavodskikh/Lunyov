#include <iostream>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <pthread.h>

using namespace std;

void* thread_f(void* arg)
{
    for (int i = 0; i < 10; ++i)
    {
        cout << i << " ";
    }
    cout << endl;
}

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Wrong num of args (" << argc-1 << " instead of 1)"
             << endl;
        exit(EXIT_FAILURE);
    }

    char* endptr = NULL;
    unsigned long num_of_cpus = strtoul(argv[1], &endptr, 10);
    vector<pthread_t> threads(num_of_cpus);

    for(int i = 0; i < num_of_cpus; ++i)
    {
        pthread_create(&threads[i], NULL, thread_f, NULL); 
    }

    for(int i = 0; i < num_of_cpus; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
