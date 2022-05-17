/*
Phu Lam} | 4/15/2022
File: lab3.cpp
Description: multi-processing with bmp images
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>    // for sbrk() and brk()
#include <math.h>      //for pow
#include <sys/mman.h>  //for mmap
#include <sys/types.h> // for forking
#include <wait.h>      // for wait call
#include <time.h>      //timer
using namespace std;

// typdefs
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

void write_file(char *argv[], BYTE *pMap, fHead &fh, iHead &ih, int start, int end);

// ./lab3 input.bmp ratRed ratGreen ratBlue output.bmp
int main(int argc, char *argv[])
{
    // reads file
    FILE *inputFile = fopen(argv[1], "rb");
    if (inputFile == NULL)
    {
        printf("Invalid file 1 input. \nPlease something like: ./colorgrading lion.bmp 0.8 1.0 0.8 result.bmp \n");
        fclose(inputFile);
        return -1;
    }

    fHead fh;
    fread(&fh.bfType, 2, 1, inputFile);
    fread(&fh.bfSize, 4, 1, inputFile); // size of the total file in bytes
    fread(&fh.bfReserved1, 2, 1, inputFile);
    fread(&fh.bfReserved2, 2, 1, inputFile);
    fread(&fh.bfOffBits, 4, 1, inputFile);

    iHead ih;
    fread(&ih.biSize, 4, 1, inputFile);   // size of the header in bytes (usually 40, must be 40 in our case)
    fread(&ih.biWidth, 4, 1, inputFile);  // number of PIXALS wide
    fread(&ih.biHeight, 4, 1, inputFile); // number of PIXALS high
    fread(&ih.biPlanes, 2, 1, inputFile);
    fread(&ih.biBitCount, 2, 1, inputFile);
    fread(&ih.biCompression, 4, 1, inputFile);
    fread(&ih.biSizeImage, 4, 1, inputFile); // image data size in bytes (excluding the header)
    fread(&ih.biXPelsPerMeter, 4, 1, inputFile);
    fread(&ih.biYPelsPerMeter, 4, 1, inputFile);
    fread(&ih.biClrUsed, 4, 1, inputFile);
    fread(&ih.biClrImportant, 4, 1, inputFile);

    BYTE *pMap = (BYTE *)mmap(NULL, ih.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    fread(pMap, ih.biSizeImage, 1, inputFile);

    // without fork
    clock_t start = clock();
    write_file(argv, pMap, fh, ih, 0, ih.biHeight);
    clock_t finish = clock();
    float sec = ((float)finish - (float)start) / CLOCKS_PER_SEC;
    cout << "without fork: " << sec << endl;

    // with fork
    start = clock();
    if (fork() == 0)
    {
        write_file(argv, pMap, fh, ih, 0, ih.biHeight / 2);
        return 0;
    }
    else
    {
        write_file(argv, pMap, fh, ih, ih.biHeight / 2, ih.biHeight);
        wait(0);
    }
    finish = clock();
    sec = ((float)finish - (float)start) / CLOCKS_PER_SEC;
    cout << "with fork: " << sec << endl;

    // writes file
    FILE *outputFile = fopen(argv[5], "wb");
    if (outputFile == NULL)
    {
        printf("Invalid file 2 input. \nPlease something like: ./colorgrading lion.bmp 0.8 1.0 0.8 result.bmp \n");
        fclose(outputFile);
        return -1;
    }
    fwrite(&fh.bfType, 2, 1, outputFile);
    fwrite(&fh.bfSize, 4, 1, outputFile);
    fwrite(&fh.bfReserved1, 2, 1, outputFile);
    fwrite(&fh.bfReserved2, 2, 1, outputFile);
    fwrite(&fh.bfOffBits, 4, 1, outputFile);
    fwrite(&ih, sizeof(ih), 1, outputFile);      // ih
    fwrite(pMap, ih.biSizeImage, 1, outputFile); // data

    // frees memmory
    fclose(inputFile);
    fclose(outputFile);
    munmap(pMap, ih.biSizeImage);

    return 0;
}

void write_file(char *argv[], BYTE *pMap, fHead &fh, iHead &ih, int start, int end)
{
    int wb = ih.biWidth * 3;
    if (wb % 4 != 0)
        wb += 4 - wb % 4;

    for (int y = start; y != end; y++)
    {
        for (int x = 0; x < ih.biWidth; x++)
        {
            BYTE b = pMap[(x * 3) + (y * wb) + 0];
            BYTE g = pMap[(x * 3) + (y * wb) + 1];
            BYTE r = pMap[(x * 3) + (y * wb) + 2];

            float fb = (((float)b / 255) * atof(argv[4])) * 255; // blue
            float fg = (((float)g / 255) * atof(argv[3])) * 255; // green
            float fr = (((float)r / 255) * atof(argv[2])) * 255; // red

            pMap[(x * 3) + (y * wb) + 0] = (BYTE)(fb);
            pMap[(x * 3) + (y * wb) + 1] = (BYTE)(fg);
            pMap[(x * 3) + (y * wb) + 2] = (BYTE)(fr);
        }
    }
}
