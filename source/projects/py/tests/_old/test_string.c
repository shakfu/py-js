#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *foo(int count) {
    char *ret = malloc(count);
    if(!ret)
        return NULL;

    for(int i = 0; i < count; ++i) 
        ret[i] = i;

    return ret;
}



void doit(char *buf, int count) {
    for(int i = 0; i < count; ++i)
        buf[i] = i;
}


int main()
{
	char txt[10] = {0};
    char txt1[50] = "Hello World";
    char* txt2 = "World Hello";
    char** p_txt = &txt2;
    doit(txt, 10);

    char *p = foo(10);
    if(p) {
        printf("p: %s length: %ld\n", p, strlen(p));
        free(p);
    }

    printf("txt1: %s length: %ld\n", txt1, strlen(txt1));
    printf("txt2: %s length: %ld\n", txt2, strlen(txt2));
    printf("p_txt: %s length: %ld\n", *p_txt, strlen(*p_txt));
    printf("txt: %s length: %ld\n", txt, strlen(txt));
}