#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0
typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

byte* negative(byte *pixels, int size);
byte* brightness(byte *pixels, int size, char di, int p);
byte* exposure(byte *pixels, int size, int p);
byte* remove_col(byte *pixels, int size, char color);
void read_bmp(const char *fileName,byte **pixels, int32 *width, int32 *height, int32 *bytesPerPixel);
void write_bmp(const char *fileName, byte *pixels, int32 width, int32 height,int32 bytesPerPixel);

int main(int argc, const char *argv[])
{
    int i, j;
    byte *pixels;
    int32 width;
    int32 height;
    int32 bytesPerPixel;
    double sum = 0;

    if(argc == 1)
    {
        printf ("Blad : nie podano parametrow");
        exit(1);
    }

    read_bmp(argv[1], &pixels, &width, &height,&bytesPerPixel);
    int size = 3*width*height;
    byte *newpixels;

    printf("%s%s\n", "nazwa pliku wejsciowego: ", argv[1]);
    printf("%s%d%s%d\n", "rozmiar obrazu: ", height, " x ", width);

    printf("%s", "wykonuje operacje: ");
    struct timeval begin, end;
    if (*argv[2] == '1'){
        printf("%s", "negatyw\n");
        gettimeofday(&begin, 0);
        newpixels = negative(pixels, size);
    } else if (*argv[2] == '2'){
        printf("%s", "zmiana jasnosci - ");
        gettimeofday(&begin, 0);
        newpixels = brightness(pixels, size, *argv[3], atoi(argv[4]));
    } else if (*argv[2] == '3'){
        printf("%s", "zmiana ekspozycji - ");
        gettimeofday(&begin, 0);
        newpixels = exposure(pixels, size, atoi(argv[3]));
    } else if (*argv[2] == '4'){
        printf("%s", "usuwanie koloru skladowego - ");
        gettimeofday(&begin, 0);
        newpixels = remove_col(pixels, size, *argv[3]);
        gettimeofday(&end, 0);
    } else {
        printf("niepoprawne parametry programu");
        exit(1);
    }

    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;
    printf("Czas: %.3f seconds.\n", elapsed);

    write_bmp(strcat(strtok(argv[1], "."), "-out.bmp"), newpixels, width, height, bytesPerPixel);
    free(pixels);
    return 0;
}

byte* negative(byte* pixels, int size){
    for(int i = 0; i < size; i++){
        pixels[i] = 255 - pixels[i];
    }
    return pixels;
}
byte* brightness(byte* pixels, int size, char di, int p){
    if (di == 'i'){
        printf("%s%d%s", "zwiększam ", p, " razy");
        for (int i=0; i<size; i++){
            pixels[i] = pixels[i] * p;
            if (pixels[i] > 255){
                pixels[i] = 255;
            }
        }
    } else if (di == 'd'){
        printf("%s%d%s", "zmniejszam ", p, " razy");
        for (int i=0; i<size; i++){
            pixels[i] = pixels[i] / p;
            if (pixels[i] < 0){
                pixels[i] = 0;
            }
        }
    } else {
        printf("\n##niepoprawne parametry funkcji##\n");
        exit(1);
    }
    return pixels;
}
byte* exposure(byte* pixels, int size, int p){
    if (p > 2 || p < 0){
        printf("\n##niepoprawne parametry funkcji##\n");
    }
    printf("%d%s", p, " razy");
    for (int i=0; i< size; i++){
        pixels[i] = 255 - (pixels[i] * 0.1 * p);
    }
    return pixels;
}
byte *remove_col(byte *pixels, int size, char color) {
    if (color == 'r'){
        printf("%s", " czerwony\n");
        for (int i; i<1000;i++){
            for (int i = 2; i< size; i += 3){
                pixels[i] = 0;
            }}
    } else if (color == 'g'){
        printf("%s", " zielony\n");
        for (int i = 1; i< size; i += 3){
            pixels[i] = 0;
        }
    } else if (color == 'b'){
        printf("%s", " niebieski\n");
        for (int i = 0; i< size; i += 3){
            pixels[i] = 0;
        }
    } else {
        printf("\n##niepoprawne parametry funkcji##\n");
    }
    return pixels;
}

void read_bmp(const char *fileName,byte **pixels, int32 *width, int32 *height, int32 *bytesPerPixel) {
    FILE *imageFile = fopen(fileName, "rb");

    int32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);

    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);

    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);

    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    *bytesPerPixel = ((int32)bitsPerPixel) / 8;

    int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(*bytesPerPixel);
    int unpaddedRowSize = (*width)*(*bytesPerPixel);
    int totalSize = unpaddedRowSize*(*height);
    *pixels = (byte*)malloc(totalSize);
    int i = 0;
    byte *currentRowPointer = *pixels+((*height-1)*unpaddedRowSize);
    for (i = 0; i < *height; i++) {
        fseek(imageFile, dataOffset+(i*paddedRowSize), SEEK_SET);
        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
        currentRowPointer -= unpaddedRowSize;
    }
    fclose(imageFile);
}


void write_bmp(const char *fileName, byte *pixels, int32 width, int32 height,int32 bytesPerPixel) {
    FILE *outputFile = fopen(fileName, "wb");
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, outputFile);
    fwrite(&BM[1], 1, 1, outputFile);

    int paddedRowSize = (int)(4 * ceil((float)width/4.0f))*bytesPerPixel;
    int32 fileSize = paddedRowSize*height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, outputFile);

    int32 reserved = 0x0000;
    fwrite(&reserved, 4, 1, outputFile);

    int32 dataOffset = HEADER_SIZE+INFO_HEADER_SIZE;
    fwrite(&dataOffset, 4, 1, outputFile);

    int32 infoHeaderSize = INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, outputFile);

    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);

    int16 planes = 1; //always 1
    fwrite(&planes, 2, 1, outputFile);

    int16 bitsPerPixel = bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, outputFile);

    int32 compression = NO_COMPRESION;
    fwrite(&compression, 4, 1, outputFile);

    int32 imageSize = width*height*bytesPerPixel;
    fwrite(&imageSize, 4, 1, outputFile);

    int32 resolutionX = 11811;
    int32 resolutionY = 11811;
    fwrite(&resolutionX, 4, 1, outputFile);
    fwrite(&resolutionY, 4, 1, outputFile);

    int32 colorsUsed = MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, outputFile);

    int32 importantColors = ALL_COLORS_REQUIRED;
    fwrite(&importantColors, 4, 1, outputFile);

    int i = 0;
    int unpaddedRowSize = width*bytesPerPixel;
    for ( i = 0; i < height; i++)
    {
        int pixelOffset = ((height - i) - 1)*unpaddedRowSize;
        fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
    }
    fclose(outputFile);
}
