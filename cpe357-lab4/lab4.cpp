// lab 4
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

// global
int fd[2];

// prototype
void sigusr1_handler(int i);

int main()
{
    pipe(fd);
    int parent_pid = getpid();
    int save_stdin = dup(STDIN_FILENO);
    int *flag = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *child_pid = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (fork() == 0)
    {
        *child_pid = getpid();
        for (;;)
        {
            *flag = 0;
            sleep(10);
            if (*flag == 0)
            {
                write(fd[1], "Inactivity detected", 20);
                kill(parent_pid, SIGUSR1);
            }
        }
        return 0;
    }

    char input[100];
    for (;;)
    {
        write(STDIN_FILENO, ">", 1);

        signal(SIGUSR1, sigusr1_handler);
        int end = read(STDIN_FILENO, input, 100);
        *flag = 1;
        input[end - 1] = '!';
        input[end] = 0;

        if (input[0] == 'q')
            break;

        printf("!%s\n", input);
        dup2(save_stdin, STDIN_FILENO);
    }

    // cleans
    close(fd[0]);
    close(fd[1]);
    munmap(flag, sizeof(flag));
    munmap(child_pid, sizeof(child_pid));
    kill(*child_pid, SIGKILL);
    wait(0);
    return 0;
}

void sigusr1_handler(int i)
{
    dup2(fd[0], STDIN_FILENO);
}
