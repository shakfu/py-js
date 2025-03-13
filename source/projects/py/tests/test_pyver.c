#include <stdio.h>
#include <Python.h>

// gcc -o test_pver test_pyver.c -I `python3-config --include`

#define _STR(x) #x
#define STR(x) _STR(x)
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define _PY_VER CONCAT(PY_MAJOR_VERSION, CONCAT(., PY_MINOR_VERSION))
#define PY_VER STR(_PY_VER)

#define VER STR(PY_MAJOR_VERSION) "." STR(PY_MINOR_VERSION)



int main()
{
	printf("pyver: " PY_VER "\n");
	printf("ver: " STR(PY_MAJOR_VERSION) "." STR(PY_MINOR_VERSION) "\n");
	printf("ver: " VER "\n");
	return 0;
}