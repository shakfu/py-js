/*

Analysis finished in 268mss

Found 3 issues

test_args.c:55: error: MEMORY_LEAK
  memory dynamically allocated by call to `malloc()` at line 52, column 25 is not reachable after line 55, column 9.
  53.
  54.       for(i = 1; i <= size; i++) {
  55. >         str = (char *)realloc(str, (v + strlen(argv[i])));
  56.           strcat(str, argv[i]);
  57.           strcat(str, " ");

test_args.c:60: error: MEMORY_LEAK
  memory dynamically allocated by call to `malloc()` at line 52, column 25 is not reachable after line 60, column 5.
  58.       }
  59.
  60. >     printf("%s\n", str);
  61.       return 0;
  62.   }

test_args.c:56: error: NULL_DEREFERENCE
  pointer `str` last assigned on line 55 could be null and is dereferenced by call to `strcat()` at line 56, column 9.
  54.       for(i = 1; i <= size; i++) {
  55.           str = (char *)realloc(str, (v + strlen(argv[i])));
  56. >         strcat(str, argv[i]);
  57.           strcat(str, " ");
  58.       }


Summary of the reports

       MEMORY_LEAK: 2
  NULL_DEREFERENCE: 1

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc == 1) {
        printf("ERROR: Expected at least 1 argument\n");
        return 0;
    }

    int i, v = 0, size = argc - 1;

    char *str = (char *)malloc(v);

    for(i = 1; i <= size; i++) {
        str = (char *)realloc(str, (v + strlen(argv[i])));
        strcat(str, argv[i]);
        strcat(str, " ");
    }

    printf("%s\n", str);
    return 0;
}
