#include "pocketpy.h"
#include <stdio.h>

/*

print_args(1, '2', 3.0, True, [None, ('a', 'b')])

a = [i for i in range(5)]
print_args(*a)

*/


#define INPUT ""

static bool print_args(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1); // single arg: `*args` tuple
    PY_CHECK_ARG_TYPE(0, tp_tuple); // check that it is

    int tuple_len = py_tuple_len(py_arg(0));

    printf("tuple_len: %d\n", tuple_len);
    for(int i = 0; i < tuple_len; i++) {
        py_Ref tuple_item = py_tuple_getitem(py_arg(0), i);
        if (!py_str(tuple_item)) {
            return false;
        }
        printf("%s\n", py_tostr(py_retval()));
    }
    py_newnone(py_retval());
    return true;
}

int main() {
    py_initialize();

    py_GlobalRef __main__ = py_getmodule("__main__");
    py_bind(__main__, "print_args(*args)", print_args);

    if (!py_exec(INPUT, "main.py", EXEC_MODE, NULL)) {
        py_printexc();
    }

    py_finalize();
    return 0;
}