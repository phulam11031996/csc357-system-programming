// lab5 - phu
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
    int fd = shm_open("sharedMem", O_RDWR | O_CREAT, 0777);
    ftruncate(fd, 100 * sizeof(char));
    char *input = (char *)mmap(NULL, 100 * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // user input
    printf(">");
    scanf("%[^\n]%*c", input);

    // to end program 1
    for (;;)
    {
        char quit[100];
        printf("type `quit` to quit: ");
        scanf("%s", quit);
        if (!strcmp(quit, "quit"))
            break;
    }

    // cleans up
    close(fd);
    shm_unlink("sharedMem");
    munmap(input, 100 * sizeof(char));
    return 0;
}