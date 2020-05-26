#include <stdio.h>



int main() {

	char txt1[50] = "Hello World";
	char *txt2 = "World Hello";
	char **p_txt = &txt2;


	printf("bye: %s length: %ld\n", txt1, strlen(txt1));
	printf("bye: %s length: %ld\n", txt2, strlen(txt2));
	printf("bye: %s length: %ld\n", *p_txt, strlen(*p_txt));
}