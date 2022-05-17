// CPE 357 - LAB 2
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;

// structure declaration
struct tagBITMAPFILEHEADER
{
  WORD bfType;      // specifies the file type
  DWORD bfSize;     // specifies the size in bytes of the bitmap file
  WORD bfReserved1; // reserved; must be 0
  WORD bfReserved2; // reserved; must be 0
  DWORD bfOffBits;  // species the offset in bytes from the bitmapfileheader to
};

struct tagBITMAPINFOHEADER
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
  DWORD biClrImportant; // number of colors that are important
};

// function declaration
void read_headers(tagBITMAPFILEHEADER &fileHeader, tagBITMAPINFOHEADER &imageHeader);
void read_pixel(BYTE *pStart);
void write_into_file(BYTE *pStart, DWORD bfOffBits, float factor);
BYTE get_contrast(BYTE rgb, float factor);

int main()
{
  tagBITMAPFILEHEADER fileHeader;
  tagBITMAPINFOHEADER imageHeader;
  read_headers(fileHeader, imageHeader);

  BYTE *pStart = (BYTE *)sbrk(fileHeader.bfSize * sizeof(BYTE));
  read_pixel(pStart);
  write_into_file(pStart, fileHeader.bfOffBits, 0.1);

  sbrk(0 - fileHeader.bfSize);

  return 0;
}

// function definition
void read_headers(tagBITMAPFILEHEADER &fileHeader, tagBITMAPINFOHEADER &imageHeader)
{
  FILE *file = fopen("flowers.bmp", "rb");
  fread(&fileHeader.bfType, sizeof(fileHeader.bfType), 1, file);
  fread(&fileHeader.bfSize, sizeof(fileHeader.bfSize), 1, file);
  fread(&fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1), 1, file);
  fread(&fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2), 1, file);
  fread(&fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits), 1, file);
  fread(&imageHeader.biSize, sizeof(imageHeader.biSize), 1, file);
  fread(&imageHeader.biWidth, sizeof(imageHeader.biWidth), 1, file);
  fread(&imageHeader.biHeight, sizeof(imageHeader.biHeight), 1, file);
  fread(&imageHeader.biPlanes, sizeof(imageHeader.biPlanes), 1, file);
  fread(&imageHeader.biBitCount, sizeof(imageHeader.biBitCount), 1, file);
  fread(&imageHeader.biCompression, sizeof(imageHeader.biCompression), 1, file);
  fread(&imageHeader.biSizeImage, sizeof(imageHeader.biSizeImage), 1, file);
  fread(&imageHeader.biXPelsPerMeter, sizeof(imageHeader.biYPelsPerMeter), 1, file);
  fread(&imageHeader.biYPelsPerMeter, sizeof(imageHeader.biYPelsPerMeter), 1, file);
  fread(&imageHeader.biClrUsed, sizeof(imageHeader.biClrUsed), 1, file);
  fread(&imageHeader.biClrImportant, sizeof(imageHeader.biClrImportant), 1, file);
  fclose(file);
}

void read_pixel(BYTE *pStart)
{
  FILE *file = fopen("flowers.bmp", "rb");

  int index = 0;
  while (!feof(file))
  {
    fread(pStart + sizeof(BYTE) * index, sizeof(BYTE), 1, file);
    index++;
  }

  fclose(file);
}

void write_into_file(BYTE *pStart, DWORD bfOffBits, float factor)
{
  FILE *file = fopen("flowers.bin", "wb");
  fwrite(pStart, bfOffBits, 1, file);

  int index = 0;
  while (&pStart[int(bfOffBits) + index] != sbrk(0))
  {
    BYTE contrast_val = get_contrast(pStart[int(bfOffBits) + index], factor);
    fwrite(&contrast_val, sizeof(BYTE), 1, file);
    index++;
  }

  fclose(file);
}

BYTE get_contrast(BYTE rgb, float factor)
{
  return (BYTE)(pow((float)rgb / 255.0, factor) * 255);
}
