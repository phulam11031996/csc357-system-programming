#include <stdio.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for (;;)
    {
        char c[1000];
        scanf("%s", c);
        char *c1 = strtok(c, " ");

        printf("%s\n", c1);

    }

    return 0;
}