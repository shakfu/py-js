#include <stdio.h>

// clang -std=c11 -o test_generics test_generics.c

void print_int(int);
void print_float(float);

#define print(x) _Generic((x), int: print_int(x), float: print_float(x))


void print_int(int n) { printf("this is an int: %i\n", n); }

void print_float(float n) { printf("this is a float: %f\n", n); }


int main(int argc, char* argv[])
{
    int i = 10;
    float f = 5.5;

    print(i);
    print(f);

    return 0;
}
