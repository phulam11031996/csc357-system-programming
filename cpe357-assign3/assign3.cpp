// assign3 - phu
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <dirent.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// def
#define BUFFERSIZE 1000
#define CHILDSIZE 9

int fd[2];

// function prototype
void findstuff(char *filename, char *startdir, char *result, int searchindir);
char **tokenize_args(char *args, char **arg_token);
void sigusr1_handler(int i);

int main()
{
    pipe(fd);
    signal(SIGUSR1, sigusr1_handler);
    int parent_pid = getpid();
    int save_stdin = dup(STDIN_FILENO);
    int *flag = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *child_pid[CHILDSIZE];
    int *pid_flag[CHILDSIZE];

    for (int i = 0; i < CHILDSIZE; i++)
        child_pid[i] = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < CHILDSIZE; i++)
        pid_flag[i] = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (;;)
    {
        char args[BUFFERSIZE] = "";
        char *arg_tokens[CHILDSIZE] = {0};

        if (*flag == 0)
            write(STDIN_FILENO, "\nfindstuff$ ", 12);

        read(STDIN_FILENO, args, BUFFERSIZE);
        dup2(save_stdin, STDIN_FILENO);

        // print: for user input
        if (*flag == 0)
            tokenize_args(args, arg_tokens);

        // print: for child overide
        if (*flag == 1 && args != NULL)
            printf("%s\n", args);

        *flag = 0;

        if (arg_tokens[0] != NULL && strcmp(arg_tokens[0], "find") == 0 && (arg_tokens[2] == NULL || (strcmp(arg_tokens[2], "-s") == 0 || strcmp(arg_tokens[2], "-f") == 0)))
        {
            if (fork() == 0)
            {
                for (int i = 0; i < CHILDSIZE; i++)
                {
                    if (*pid_flag[i] == 0)
                    { // finds the first zero flag
                        *pid_flag[i] = 1;
                        *child_pid[i] = getpid();
                        break;
                    }

                    if (i == 8 && *pid_flag[8] == 1)
                    { // case: 10 processes are runnung
                        write(fd[1], "\n>10 processes are running<", 27);
                        kill(parent_pid, SIGUSR1); // overide parent
                        close(fd[0]);
                        close(fd[1]);
                        *flag = 1;
                        return 0;
                    }
                }

                char dir[BUFFERSIZE];
                char result[BUFFERSIZE] = "";
                char *arg_tokens_dir[CHILDSIZE] = {0};

                if (arg_tokens[2] == NULL)
                { // fron curr and no sub
                    getcwd(dir, BUFFERSIZE);
                    findstuff(arg_tokens[1], dir, result, 0);
                }
                else if (strcmp(arg_tokens[2], "-f") == 0)
                { // from root and sub
                    strcpy(dir, "/");
                    findstuff(arg_tokens[1], dir, result, 1);
                }
                else if (strcmp(arg_tokens[2], "-s") == 0)
                { // from cwd and sub
                    getcwd(dir, BUFFERSIZE);
                    findstuff(arg_tokens[1], dir, result, 1);
                }

                sleep(50);

                *flag = 1;

                printf("\n");
                if (strlen(result) == 0)
                    write(fd[1], "\n>found-nothing<", 16);
                else
                    write(fd[1], result, BUFFERSIZE);

                kill(parent_pid, SIGUSR1); // overide parent
                close(fd[0]);
                close(fd[1]);
                return 0;
            }
        }
        else if (arg_tokens[0] != NULL && (strcmp(arg_tokens[0], "quit") == 0 || strcmp(arg_tokens[0], "q") == 0))
        {
            for (int i = 0; i < CHILDSIZE; i++)
            {
                if (*pid_flag[i] == 1)
                {
                    kill(*child_pid[i], SIGKILL); // kills
                    waitpid(*child_pid[i], NULL, WNOHANG);
                    *pid_flag[i] = 0;
                }
                munmap(child_pid[i], sizeof(child_pid[i]));
                munmap(pid_flag[i], sizeof(pid_flag[i]));
            }
            munmap(flag, sizeof(flag));
            close(fd[0]);
            close(fd[1]);
            wait(0);
            break;
        }

        for (int i = 0; i < CHILDSIZE; i++)
        {
            int status = waitpid(*child_pid[i], NULL, WNOHANG);
            if (status > 0) // sets flag to zero if the process is done
                *pid_flag[i] = 0;
        }
    }

    return 0;
}

// function def

//////////////////////////////////////////////////////////////////////////////
// DO: Finds matching file name and at their directory to char *result      //
// ARGS:                                                                    //
// RETURN:                                                                  //
//////////////////////////////////////////////////////////////////////////////
void findstuff(char *filename, char *startdir, char *result, int searchsubdir)
{
    DIR *dir = opendir(startdir);
    struct dirent *entry;

    if (dir == NULL)
        return;

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(entry->d_name, filename) == 0)
        { // case: match file name
            if (strcmp(result, "") != 0)
                strcat(result, "\n");
            strcat(result, filename);
            strcat(result, "\t");
            strcat(result, startdir);
        }

        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && searchsubdir == 1)
        {
            if (entry->d_type == DT_DIR)
            { // case: file is a directory
                char newdir[1000];
                strcpy(newdir, startdir);
                if (strcmp(startdir, "/") != 0)
                    strcat(newdir, "/");
                strcat(newdir, entry->d_name);
                findstuff(filename, newdir, result, searchsubdir);
            }
        }
    }
    closedir(dir);
}

//////////////////////////////////////////////////////////////////////////////
// DO: Parsing string into pointers. Resulting string are seperate by space //
// ARGS:                                                                    //
// RETURN:                                                                  //
//////////////////////////////////////////////////////////////////////////////
char **tokenize_args(char *args, char **arg_tokens)
{
    int num_args = 0;
    strtok(args, "\n");
    char *token = strtok(args, " ");

    while (token != NULL)
    {
        arg_tokens[num_args] = token;
        token = strtok(NULL, " ");
        num_args++;
    }
    return arg_tokens;
}

//////////////////////////////////////////////////////////////////////////////
// DO:                                                                      //
// ARGS:                                                                    //
// RETURN:                                                                  //
//////////////////////////////////////////////////////////////////////////////
void sigusr1_handler(int i)
{
    dup2(fd[0], STDIN_FILENO);
}
