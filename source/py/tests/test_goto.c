#include <stdio.h>


int main() {

    int x = 10;


hello:
    x++;
    printf("hello: %d\n", x);

bye:
    x++;
    printf("bye: %d\n", x);

    return 0;
}

