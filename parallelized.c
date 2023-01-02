#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
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

    int ierr = MPI_Init(&argc, &argv);
    int procid, numprocs;

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    if(argc == 1)
    {
        printf ("Blad : nie podano parametrow");
        exit(1);
    }

    read_bmp(argv[1], &pixels, &width, &height,&bytesPerPixel);
    int size = 3*width*height;

    unsigned int partition = size / numprocs;

    byte *newpixels;
    printf("%s%s\n", "nazwa pliku wejsciowego: ", argv[1]);
    printf("%s%d%s%d\n", "rozmiar obrazu: ", height, " x ", width);

    printf("%s", "wykonuje operacje: ");
    struct timeval begin, end;
    if (*argv[2] == '1'){

        gettimeofday(&begin, 0);
        printf("%s", "negatyw\n");
        for(int i = 0; i < size; i++){
            if (i% numprocs != procid) continue;
            pixels[i] = 255 - pixels[i];
        }
        gettimeofday(&end, 0);

    } else if (*argv[2] == '2'){

        gettimeofday(&begin, 0);
        printf("%s", "zmiana jasnosci - ");
        char di = argv[3];
        int p = atoi(argv[4]);
        if (di == 'i'){
            printf("%s%d%s", "zwiększam ", p, " razy");
            for (int i=0; i<size; i++){
                if (i% numprocs != procid) continue;
                pixels[i] = pixels[i] * p;
                if (pixels[i] > 255){
                    pixels[i] = 255;
                }
            }
        } else if (di == 'd'){
            printf("%s%d%s", "zmniejszam ", p, " razy");
            for (int i=0; i<size; i++){
                if (i% numprocs != procid) continue;
                pixels[i] = pixels[i] / p;
                if (pixels[i] < 0){
                    pixels[i] = 0;
                }
            }
        } else {
            printf("\n##niepoprawne parametry funkcji##\n");
            exit(1);
        }
        gettimeofday(&end, 0);
    } else if (*argv[2] == '3'){
        gettimeofday(&begin, 0);
        printf("%s", "zmiana ekspozycji - ");
        int p = atoi(argv[3]);
        if (p > 2 || p < 0){
            printf("\n##niepoprawne parametry funkcji##\n");
        }
        printf("%d%s", p, " razy");
        for (int i=0; i< size; i++){
            if (i% numprocs != procid) continue;
            pixels[i] = 255 - (pixels[i] * 0.1 * p);
        }
        gettimeofday(&end, 0);
    } else if (*argv[2] == '4'){
        printf("%s", "usuwanie koloru skladowego - ");
        gettimeofday(&begin, 0);
        char color = argv[3];
        if (color == 'r'){
            printf("%s", " czerwony\n");
            for (int j=0; j<1000;j++){
                for (int i = 2; i< size; i += 3){
                    if (i% numprocs != procid) continue;
                    pixels[i] = 0;
                }}
        } else if (color == 'g'){
            printf("%s", " zielony\n");
            for (int i = 1; i< size; i += 3){
                if (i% numprocs != procid) continue;
                pixels[i] = 0;
            }
        } else if (color == 'b'){
            printf("%s", " niebieski\n");
            for (int i = 0; i< size; i += 3){
                if (i% numprocs != procid) continue;
                pixels[i] = 0;
            }
        } else {
            printf("\n##niepoprawne parametry funkcji##\n");
        }
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
