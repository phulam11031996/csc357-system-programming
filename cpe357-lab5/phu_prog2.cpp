// lab 5 - phu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

#include <fcntl.h>

int main()
{
    // inits
    int fd = shm_open("sharedMem", O_RDWR, 0777);
    char *p = (char *)mmap(NULL, 100 * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // prints input from program 1
    if (fd != -1)
    {
        printf("connected\n");
        for (;;) // check if the user enter input yet
        {
            sleep(1);
            if (*p != '\0')
            {
                printf("%s\n", p);
                break;
            }
        }
    }
    else
        printf("fails to connect!\n");

    // cleans up
    close(fd);
    shm_unlink("sharedMem");
    munmap(p, 100 * sizeof(char));
    return 0;
}