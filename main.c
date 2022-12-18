#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void load_image();
void negative();
void brightness();
void exposure();
void remove_comp();

int main(int argc, char *argv[]) {

    if(argc == 1)
    {
        printf ("Blad : nie podano parametrow");
        exit(1);
    }
}