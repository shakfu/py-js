#include <stdio.h>
#include <stdarg.h>


void printf2(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main() {

	printf("printf: hello %d\n", 1);
	printf2("printf2: hello %d\n", 1);

}