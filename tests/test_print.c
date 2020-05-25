#include <stdio.h>
#include <stdarg.h>


void printf2(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


void sprintf2(char *buf, char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);
}

void test_sprintf2(void) {
    char buff[100];
    sprintf2(buff, "Hello, %s, aged %d", "Jo", 27);
    printf("%s\n", buff);

}


int main() {
	printf("printf: hello %d\n", 1);
	printf2("printf2: hello %d\n", 1);
	test_sprintf2();
}