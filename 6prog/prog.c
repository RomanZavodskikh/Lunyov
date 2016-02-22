#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const int SLEEP_TIME = 1;

FILE* stdlog;

pid_t CHILD_ID = 0;
pid_t PARENT_ID = 0;
unsigned char BIT_IN_BYTE = 0;
unsigned char CUR_BYTE = 0;
int fd = -1;
unsigned int NEED_TO_WAIT = 1;
unsigned int WAIT_TIME = 1;
sigset_t CHILD_MASK;
sigset_t PARENT_MASK;

void kill_err(pid_t pid, int signal)
{
    if (kill(pid, signal) == -1)
    {
        perror("kill");
        exit(EXIT_FAILURE);
    }
}

void write_err(int fd, const void* buf, size_t count)
{
    if ( write(fd, buf, count) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

void child_handler(int signum)
{
    //do nothing
}

void parent_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        //do nothing
    }
    else if (signum == SIGUSR2)
    {
        unsigned char cur_bit = 1U << BIT_IN_BYTE;
        CUR_BYTE |= cur_bit;
    }
    else if (signum == SIGCHLD)
    {
        NEED_TO_WAIT = 0;
    }
}

void child_work(int argc, char** argv)
{
    //prohibit every prohibitable signal
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    //opening the file
    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    //creating handler 
    PARENT_ID = getppid();
    struct sigaction act;
    act.sa_handler = child_handler;
    sigaction(SIGUSR1, &act, NULL);
    act.sa_handler = SIG_DFL;
    sigaction(SIGALRM, &act, NULL);

    //catching signals
    BIT_IN_BYTE = -1;
    while (NEED_TO_WAIT)
    {
        //making BIT_IN_BYTE exist in [0..7]
        BIT_IN_BYTE++;
        if (BIT_IN_BYTE == 8)
        {
            BIT_IN_BYTE = 0;
        }

        // if CUR_BYTE hadn't begun, read it from file
        ssize_t readen = 1;
        if (BIT_IN_BYTE == 0)
        {
            readen = read(fd, &CUR_BYTE, 1);
        }
        if (readen == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if (readen == 0)
        {
            NEED_TO_WAIT = 0;
            return;
        }

        //senging current bit to parent
        unsigned char cur_bit = (CUR_BYTE & (1U << BIT_IN_BYTE))
            >> BIT_IN_BYTE;
        alarm(WAIT_TIME);
        if ( cur_bit == 0)
        {
            kill_err(PARENT_ID, SIGUSR1);
        }
        else if ( cur_bit == 1)
        {
            kill_err(PARENT_ID, SIGUSR2);
        }
        else
        {
            fprintf(stderr, "Attention! Third bit (%i) is detected!\n",
                cur_bit);
            fprintf(stderr, "Leaving...\n");
            exit(EXIT_FAILURE);
        }

        //waiting parent to response
        sigsuspend(&CHILD_MASK);
    }
}

void parent_work (int argc, char** argv, pid_t cpid)
{
    //creating handler
    CHILD_ID = cpid;
    struct sigaction act;
    act.sa_handler = parent_handler;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    act.sa_handler = SIG_IGN;
    sigaction(SIGALRM, &act, NULL);

    //catching signals
    sigsuspend(&PARENT_MASK);
    while (NEED_TO_WAIT)
    {
        //making BIT_IN_BYTE exist in [0..7]
        BIT_IN_BYTE++;
        if( BIT_IN_BYTE == 8)
        {
            write_err(STDOUT_FILENO, &CUR_BYTE, 1);
            BIT_IN_BYTE = 0;
            CUR_BYTE = 0;
        }

        //claim new bit from child
        kill_err(CHILD_ID, SIGUSR1);

        //waiting for response from child
        sigsuspend(&PARENT_MASK);
    }
}

int main (int argc, char** argv)
{
    errno = 0;
    stdlog = stderr;
    //creating CHILD_MASK ...
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGALRM);
    CHILD_MASK = set;
    // ... and PARENT_MASK
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGUSR2);
    sigdelset(&set, SIGCHLD);
    PARENT_MASK = set;

    //prohibit every prohibitable signal
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    //deleting any actions
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGALRM, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);

    //handling error of wrong num of args
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./Prog <filename>\n");
        exit(EXIT_FAILURE);
    }

    //forking
    int pid = fork();
    if (pid == 0)
    {
        child_work(argc, argv);
    }
    else if (pid > 0)
    {
        parent_work(argc, argv, pid);
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return 0;
}
