// assign2 - phu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // for ceil and floor

#include <sys/mman.h> // for mmap and munmap

// type and struct definition
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} fHead;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} iHead;

// function prototype
void write_file(char *argv[], fHead fhBig, iHead ihBig, BYTE *pImageBig, fHead fhSmall, iHead ihSmall, BYTE *pImageSmall, BYTE *pImageR);
BYTE get_color(BYTE *pImage, float x_2, float y_2, int wbS, int zeroOneTwo);

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("Wrong or missing parameters.\n");
        printf("Enter something like [programname] [imagefile1] [imagefile2] [ratio] [outputfile]\n");
        return -1;
    }

    if (!(atof(argv[3]) >= 0 && atof(argv[3]) <= 1))
    {
        printf("[ratio] must be with in 0 and 1.\n ");
        return -1;
    }

    fHead fh1;
    iHead ih1;

    fHead fh2;
    iHead ih2;

    // read file 1
    FILE *inputFile = fopen(argv[1], "rb");

    if (inputFile == NULL)
    {
        printf("Invalid file 2 input.\n");
        fclose(inputFile);
        return -1;
    }
    fread(&fh1.bfType, 2, 1, inputFile);
    fread(&fh1.bfSize, 4, 1, inputFile);
    fread(&fh1.bfReserved1, 2, 1, inputFile);
    fread(&fh1.bfReserved2, 2, 1, inputFile);
    fread(&fh1.bfOffBits, 4, 1, inputFile);

    fread(&ih1.biSize, 4, 1, inputFile);
    fread(&ih1.biWidth, 4, 1, inputFile);
    fread(&ih1.biHeight, 4, 1, inputFile);
    fread(&ih1.biPlanes, 2, 1, inputFile);
    fread(&ih1.biBitCount, 2, 1, inputFile);
    fread(&ih1.biCompression, 4, 1, inputFile);
    fread(&ih1.biSizeImage, 4, 1, inputFile);
    fread(&ih1.biXPelsPerMeter, 4, 1, inputFile);
    fread(&ih1.biYPelsPerMeter, 4, 1, inputFile);
    fread(&ih1.biClrUsed, 4, 1, inputFile);
    fread(&ih1.biClrImportant, 4, 1, inputFile);

    BYTE *pImage1 = (BYTE *)mmap(NULL, ih1.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    fread(pImage1, ih1.biSizeImage, 1, inputFile);
    fclose(inputFile);

    // read file 2
    inputFile = fopen(argv[2], "rb");

    if (inputFile == NULL)
    {
        printf("Invalid file 1 input.\n");
        fclose(inputFile);
        return -1;
    }
    fread(&fh2.bfType, 2, 1, inputFile);
    fread(&fh2.bfSize, 4, 1, inputFile);
    fread(&fh2.bfReserved1, 2, 1, inputFile);
    fread(&fh2.bfReserved2, 2, 1, inputFile);
    fread(&fh2.bfOffBits, 4, 1, inputFile);

    fread(&ih2.biSize, 4, 1, inputFile);
    fread(&ih2.biWidth, 4, 1, inputFile);
    fread(&ih2.biHeight, 4, 1, inputFile);
    fread(&ih2.biPlanes, 2, 1, inputFile);
    fread(&ih2.biBitCount, 2, 1, inputFile);
    fread(&ih2.biCompression, 4, 1, inputFile);
    fread(&ih2.biSizeImage, 4, 1, inputFile);
    fread(&ih2.biXPelsPerMeter, 4, 1, inputFile);
    fread(&ih2.biYPelsPerMeter, 4, 1, inputFile);
    fread(&ih2.biClrUsed, 4, 1, inputFile);
    fread(&ih2.biClrImportant, 4, 1, inputFile);

    BYTE *pImage2 = (BYTE *)mmap(NULL, ih2.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    fread(pImage2, ih2.biSizeImage, 1, inputFile);
    fclose(inputFile);

    // checks which image has a larger width
    if (ih1.biWidth > ih2.biWidth)
    {
        BYTE *pImageR = (BYTE *)mmap(NULL, ih1.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        write_file(argv, fh1, ih1, pImage1, fh2, ih2, pImage2, pImageR);
        munmap(pImageR, ih1.biSizeImage);
    }
    else
    {
        BYTE *pImageR = (BYTE *)mmap(NULL, ih2.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        write_file(argv, fh2, ih2, pImage2, fh1, ih1, pImage1, pImageR);
        munmap(pImageR, ih2.biSizeImage);
    }

    munmap(pImage1, ih1.biSizeImage);
    munmap(pImage2, ih2.biSizeImage);

    return 0;
}

void write_file(char *argv[], fHead fhBig, iHead ihBig, BYTE *pImageBig, fHead fhSmall, iHead ihSmall, BYTE *pImageSmall, BYTE *pImageR)
{
    FILE *outputFile = fopen(argv[4], "wb");

    if (outputFile == NULL)
    {
        printf("Invalid file 2 input.\n");
        fclose(outputFile);
        return;
    }

    // writes ih and fh
    fwrite(&fhBig.bfType, 2, 1, outputFile);
    fwrite(&fhBig.bfSize, 4, 1, outputFile);
    fwrite(&fhBig.bfReserved1, 2, 1, outputFile);
    fwrite(&fhBig.bfReserved2, 2, 1, outputFile);
    fwrite(&fhBig.bfOffBits, 4, 1, outputFile);
    fwrite(&ihBig, sizeof(ihBig), 1, outputFile);

    // padding for big image
    int wbB = ihBig.biWidth * 3;
    if (wbB % 4 != 0)
        wbB += 4 - wbB % 4;

    // padding for small image
    int wbS = ihSmall.biWidth * 3;
    if (wbS % 4 != 0)
        wbS += 4 - wbS % 4;

    for (int y = 0; y != ihBig.biHeight; y++)
    {
        for (int x = 0; x < ihBig.biWidth; x++)
        {
            // cals coordinates
            float x_2 = x * (float)ihSmall.biWidth / ihBig.biWidth;
            float y_2 = y * (float)ihSmall.biHeight / ihBig.biHeight;

            // cals rbg for small image
            BYTE bS = get_color(pImageSmall, x_2, y_2, wbS, 0);
            BYTE gS = get_color(pImageSmall, x_2, y_2, wbS, 1);
            BYTE rS = get_color(pImageSmall, x_2, y_2, wbS, 2);

            // gets rbg for big image
            BYTE bB = pImageBig[(x * 3) + (y * wbB) + 0];
            BYTE gB = pImageBig[(x * 3) + (y * wbB) + 1];
            BYTE rB = pImageBig[(x * 3) + (y * wbB) + 2];

            // mixes
            BYTE b = bB * atof(argv[3]) + bS * (1 - atof(argv[3]));
            BYTE g = gB * atof(argv[3]) + gS * (1 - atof(argv[3]));
            BYTE r = rB * atof(argv[3]) + rS * (1 - atof(argv[3]));

            // assign
            pImageR[(x * 3) + (y * wbB) + 0] = b;
            pImageR[(x * 3) + (y * wbB) + 1] = g;
            pImageR[(x * 3) + (y * wbB) + 2] = r;
        }
    }

    fwrite(pImageR, ihBig.biSizeImage, 1, outputFile);
    fclose(outputFile);
}

BYTE get_color(BYTE *pImage, float x_2, float y_2, int wbS, int zeroOneTwo)
{
    // gets 4 pixels
    BYTE lef_upper = pImage[(int)(floor(x_2) * 3 + (ceil(y_2) * wbS) + zeroOneTwo)];
    BYTE rig_upper = pImage[(int)(ceil(x_2) * 3 + (ceil(y_2) * wbS) + zeroOneTwo)];
    BYTE lef_lower = pImage[(int)(floor(x_2) * 3 + (floor(y_2) * wbS) + zeroOneTwo)];
    BYTE rig_lower = pImage[(int)(ceil(x_2) * 3 + (floor(y_2) * wbS) + zeroOneTwo)];

    float dx = x_2 - (int)x_2;
    float dy = y_2 - (int)y_2;

    // interpolation
    BYTE lef = lef_upper * (1 - dy) + lef_lower * dy;
    BYTE rig = rig_upper * (1 - dy) + rig_lower * dy;
    BYTE color = lef * (1 - dx) + rig * dx;

    return color;
}