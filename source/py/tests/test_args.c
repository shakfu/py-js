/*

Analysis finished in 314mss

No issues found

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 20

int main(int argc, char *argv[]) {

    char buffer[BUFFER_SIZE];
    int total_input_length = 0;

    if(argc == 1) {
        printf("ERROR: Expected at least 1 argument\n");
        return 0;
    }

    for (int i=1; i <= argc-1; i++) {
        total_input_length += strlen(argv[i]);
    }

    if (total_input_length > BUFFER_SIZE-1) {
        printf("input length of %d > available buffer length of %d by %d characters.\n",
        total_input_length, BUFFER_SIZE-1, total_input_length - (BUFFER_SIZE-1));
        exit(1);
    }

    // copy first
    strncpy(buffer, argv[1], BUFFER_SIZE-1);
    buffer[BUFFER_SIZE-1] = '\0';

    for(int i = 2; i <= argc-1; i++) {
        printf("%d.1 strlen(buffer): %ld\n", i, strlen(buffer));
        strncat(buffer, " ", BUFFER_SIZE - strlen(buffer) - 1);
        printf("%d.2 strlen(buffer): %ld\n", i, strlen(buffer));
        strncat(buffer, argv[i], BUFFER_SIZE - strlen(buffer) - 1);
    }

    printf("final strlen(buffer): %ld\n", strlen(buffer));
    printf("%s\n", buffer);
    return 0;
}
