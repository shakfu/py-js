// from: https://stackoverflow.com/questions/71641582/flattening-a-2d-array-in-c-without-malloc-etc
#include <stdio.h>
#include <stdlib.h>

#define ROWS    5
#define COLS    15

int main() {
    unsigned short i, j;
    int ar[ROWS][COLS];
    int flat_array [ROWS * COLS];
    puts ("2D array: ");
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            ar[i][j] = rand() % 10;
            flat_array[COLS * i + j] = ar[i][j];
            printf ("%d ", ar[i][j]);
        }
        printf ("\n");
    }
    printf("\nFlattened Array: \n");
    for (int fi = 0; fi < ROWS * COLS; )
        printf ("%d ", flat_array[fi++]);

    return 0;
}

