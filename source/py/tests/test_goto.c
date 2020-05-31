#include <stdio.h>

int main()
{

    int x = 10;

    // fall through with no return!

hello:
    x++;
    printf("hello: %d\n", x);

bye:
    x++;
    printf("bye: %d\n", x);

    return 0;
}
