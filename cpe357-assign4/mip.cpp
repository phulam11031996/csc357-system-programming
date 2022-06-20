// assign4 - phu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <wait.h>

int main(int argc, char *argv[])
{
    char *arguments[4];

    arguments[0] = (char *)malloc(100);
    arguments[1] = (char *)malloc(100);
    arguments[2] = (char *)malloc(100);
    arguments[3] = NULL;

    strcpy(arguments[0], argv[1]);
    strcpy(arguments[1], argv[2]);
    
    int n = atoi(argv[2]);
    char exe[100];
    sprintf(exe, "./%s", argv[1]);

    for (int i = 0; i < n; i++)
    {
        sprintf(arguments[2], "%d", i);
        if (fork() == 0)
        {
            execv(exe, arguments);
            printf("couldn't do execv with %s\n", exe);
            return 0;
        }
    }

    wait(0);
    free(arguments[0]);
    free(arguments[1]);
    free(arguments[2]);

    return 0;
}
