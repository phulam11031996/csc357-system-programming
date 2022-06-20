// assign 5 - phu
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

// typdef
#define DIMENSION_XY 300
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

// structs
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;      // specifies the file type
    DWORD bfSize;     // specifies the size in bytes of the bitmap file
    WORD bfReserved1; // reserved; must be 0
    WORD bfReserved2; // reserved; must be 0
    DWORD bfOffBits;  // species the offset in bytes from the bitmapfileheader to the bitmap bits
} fHead;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;         // specifies the number of bytes required by the struct
    LONG biWidth;         // specifies width in pixels
    LONG biHeight;        // species height in pixels
    WORD biPlanes;        // specifies the number of color planes, must be 1
    WORD biBitCount;      // specifies the number of bit per pixel
    DWORD biCompression;  // spcifies the type of compression
    DWORD biSizeImage;    // size of image in bytes
    LONG biXPelsPerMeter; // number of pixels per meter in x axis
    LONG biYPelsPerMeter; // number of pixels per meter in y axis
    DWORD biClrUsed;      // number of colors used by th ebitmap
    DWORD biClrImportant; // number of colors that

} iHead;

/////////////////////////////////////////////////////////////////////////////////////
//                                                                                 //
//                                                                                 //
//                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////
void quadratic_matrix_multiplication_parallel(int par_id, int par_count, float *A, float *B, float *C)
{
    int start = par_id * (floor(DIMENSION_XY / par_count));
    int end;
    if (par_id == par_count - 1)
        end = DIMENSION_XY;
    else
        end = start + (floor(DIMENSION_XY / par_count));

    for (int a = start; a < end; a++)              // over all cols a
        for (int b = 0; b < DIMENSION_XY; b++)     // over all rows b
            for (int c = 0; c < DIMENSION_XY; c++) // over all rows/cols left
                C[a + b * DIMENSION_XY] += A[c + b * DIMENSION_XY] * B[a + c * DIMENSION_XY] * 255 * 0.027;
}

/////////////////////////////////////////////////////////////////////////////////////
//                                                                                 //
//                                                                                 //
//                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////
void synch(int par_id, int par_count, int *ready, int ri)
{
    ready[par_id]++;
    int leave = 1;

    while (leave)
    {
        leave = 0;
        for (int i = 0; i < par_count; i++)
            if (ready[i] <= ri)
                leave = 1;
    }
}

int main(int argc, char *argv[])
{
    int par_id;    // the parallel ID of this process
    int par_count; // the amount of processes
    int *ready;    // needed for synch

    float *red1, *blu1, *gre1;
    float *red2, *blu2, *gre2;
    float *redR, *bluR, *greR;

    iHead iHead;
    fHead fHead;

    BYTE *pImage1;
    BYTE *pImage2;
    BYTE *pImageR;

    if (argc != 3)
    {
        printf("no shared\n");
    }
    else
    {
        par_count = atoi(argv[1]);
        par_id = atoi(argv[2]);
    }
    if (par_count == 1)
    {
        printf("only one process\n");
    }
    int fd[15];
    if (par_id == 0)
    {
        // FILE *inputImage = fopen("f0.bmp", "rb");
        FILE *inputImage = fopen("f1.bmp", "rb");
        if (inputImage == NULL)
        {
            printf("Input Image Doesn't Exist.\n");
            fclose(inputImage);
            return -1;
        }
        fread(&fHead.bfType, 2, 1, inputImage);
        fread(&fHead.bfSize, 4, 1, inputImage);
        fread(&fHead.bfReserved1, 2, 1, inputImage);
        fread(&fHead.bfReserved2, 2, 1, inputImage);
        fread(&fHead.bfOffBits, 4, 1, inputImage);

        fread(&iHead.biSize, 4, 1, inputImage);
        fread(&iHead.biWidth, 4, 1, inputImage);
        fread(&iHead.biHeight, 4, 1, inputImage);
        fread(&iHead.biPlanes, 2, 1, inputImage);
        fread(&iHead.biBitCount, 2, 1, inputImage);
        fread(&iHead.biCompression, 4, 1, inputImage);
        fread(&iHead.biSizeImage, 4, 1, inputImage);
        fread(&iHead.biXPelsPerMeter, 4, 1, inputImage);
        fread(&iHead.biYPelsPerMeter, 4, 1, inputImage);
        fread(&iHead.biClrUsed, 4, 1, inputImage);
        fread(&iHead.biClrImportant, 4, 1, inputImage);

        pImage1 = (BYTE *)mmap(NULL, iHead.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        pImage2 = (BYTE *)mmap(NULL, iHead.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        pImageR = (BYTE *)mmap(NULL, iHead.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

        fread(pImage1, iHead.biSizeImage, 1, inputImage);

        inputImage = fopen("f2.bmp", "rb");
        if (inputImage == NULL)
        {
            printf("Input Image Doesn't Exist.\n");
            fclose(inputImage);
            return -1;
        }

        fseek(inputImage, sizeof(fHead.bfOffBits), SEEK_SET);
        fread(pImage2, iHead.biSizeImage, 1, inputImage);
        fclose(inputImage);

        fd[0] = shm_open("ready", O_RDWR | O_CREAT, 0777);
        fd[1] = shm_open("red1", O_RDWR | O_CREAT, 0777);
        fd[2] = shm_open("red2", O_RDWR | O_CREAT, 0777);
        fd[3] = shm_open("redR", O_RDWR | O_CREAT, 0777);
        fd[4] = shm_open("gre1", O_RDWR | O_CREAT, 0777);
        fd[5] = shm_open("gre2", O_RDWR | O_CREAT, 0777);
        fd[6] = shm_open("greR", O_RDWR | O_CREAT, 0777);
        fd[7] = shm_open("blu1", O_RDWR | O_CREAT, 0777);
        fd[8] = shm_open("blu2", O_RDWR | O_CREAT, 0777);
        fd[9] = shm_open("bluR", O_RDWR | O_CREAT, 0777);

        ftruncate(fd[0], sizeof(int) * par_count);
        ftruncate(fd[1], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[2], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[3], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[4], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[5], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[6], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[7], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[8], DIMENSION_XY * DIMENSION_XY * sizeof(float));
        ftruncate(fd[9], DIMENSION_XY * DIMENSION_XY * sizeof(float));

        ready = (int *)mmap(NULL, sizeof(int) * par_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
        red1 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
        red2 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
        redR = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
        gre1 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[4], 0);
        gre2 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], 0);
        greR = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[6], 0);
        blu1 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[7], 0);
        blu2 = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[8], 0);
        bluR = (float *)mmap(NULL, DIMENSION_XY * DIMENSION_XY * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[9], 0);

        int wb = DIMENSION_XY * 3;
        if (wb % 4 != 0)
            wb += 4 - wb % 4;

        for (int y = 0; y != DIMENSION_XY; y++)
        {
            for (int x = 0; x < DIMENSION_XY; x++)
            {
                red1[y + x * DIMENSION_XY] = pImage1[(x * 3) + (y * wb) + 2] / 255.0;
                red2[y + x * DIMENSION_XY] = pImage2[(x * 3) + (y * wb) + 2] / 255.0;

                gre1[y + x * DIMENSION_XY] = pImage1[(x * 3) + (y * wb) + 1] / 255.0;
                gre2[y + x * DIMENSION_XY] = pImage2[(x * 3) + (y * wb) + 1] / 255.0;

                blu1[y + x * DIMENSION_XY] = pImage1[(x * 3) + (y * wb) + 0] / 255.0;
                blu2[y + x * DIMENSION_XY] = pImage2[(x * 3) + (y * wb) + 0] / 255.0;
            }
        }
    }
    else
    {
        sleep(2);

        fd[0] = shm_open("ready", O_RDWR, 0777);
        fd[1] = shm_open("red1", O_RDWR, 0777);
        fd[2] = shm_open("red2", O_RDWR, 0777);
        fd[3] = shm_open("redR", O_RDWR, 0777);
        fd[4] = shm_open("gre1", O_RDWR, 0777);
        fd[5] = shm_open("gre2", O_RDWR, 0777);
        fd[6] = shm_open("greR", O_RDWR, 0777);
        fd[7] = shm_open("blu1", O_RDWR, 0777);
        fd[8] = shm_open("blu2", O_RDWR, 0777);
        fd[9] = shm_open("bluR", O_RDWR, 0777);

        ready = (int *)mmap(NULL, sizeof(int) * par_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
        red1 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
        red2 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
        redR = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
        gre1 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[4], 0);
        gre2 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], 0);
        greR = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[6], 0);
        blu1 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[7], 0);
        blu2 = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[8], 0);
        bluR = (float *)mmap(NULL, sizeof(float) * DIMENSION_XY * DIMENSION_XY, PROT_READ | PROT_WRITE, MAP_SHARED, fd[9], 0);
    }

    synch(par_id, par_count, ready, 0);

    clock_t start;
    if (par_id == 0)
        start = clock();

    quadratic_matrix_multiplication_parallel(par_id, par_count, red1, red2, redR);
    quadratic_matrix_multiplication_parallel(par_id, par_count, gre1, gre2, greR);
    quadratic_matrix_multiplication_parallel(par_id, par_count, blu1, blu2, bluR);

    synch(par_id, par_count, ready, 1);

    if (par_id == 0)
    {
        clock_t finish = clock();
        float sec = ((float)finish - (float)start) / CLOCKS_PER_SEC;
        printf("time: %f\n", sec);
    }


    if (par_id == 0)
    {

        FILE *outputImage = fopen("result.bmp", "wb");
        if (outputImage == NULL)
        {
            printf("Can not open file.\n");
            fclose(outputImage);
            return -1;
        }
        fwrite(&fHead.bfType, 2, 1, outputImage);
        fwrite(&fHead.bfSize, 4, 1, outputImage);
        fwrite(&fHead.bfReserved1, 2, 1, outputImage);
        fwrite(&fHead.bfReserved2, 2, 1, outputImage);
        fwrite(&fHead.bfOffBits, 4, 1, outputImage);
        fwrite(&iHead, sizeof(iHead), 1, outputImage); // ih

        int wb = DIMENSION_XY * 3;
        if (wb % 4 != 0)
            wb += 4 - wb % 4;

        for (int y = 0; y != DIMENSION_XY; y++)
        {
            for (int x = 0; x < DIMENSION_XY; x++)
            {
                pImageR[(x * 3) + (y * wb) + 0] = bluR[y + x * DIMENSION_XY];
                pImageR[(x * 3) + (y * wb) + 1] = greR[y + x * DIMENSION_XY];
                pImageR[(x * 3) + (y * wb) + 2] = redR[y + x * DIMENSION_XY];
            }
        }

        fwrite(pImageR, iHead.biSizeImage, 1, outputImage);

        close(fd[0]);
        close(fd[1]);
        close(fd[2]);
        close(fd[3]);
        close(fd[4]);
        close(fd[5]);
        close(fd[6]);
        close(fd[7]);
        close(fd[8]);
        close(fd[9]);

        shm_unlink("red1");
        shm_unlink("red2");
        shm_unlink("redR");

        shm_unlink("gre1");
        shm_unlink("gre2");
        shm_unlink("greR");

        shm_unlink("blu1");
        shm_unlink("blu2");
        shm_unlink("bluR");

        shm_unlink("ready");

        return 0;
    }
}
