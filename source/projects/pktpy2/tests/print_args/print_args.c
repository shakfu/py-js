#include <stdio.h>
#include "pocketpy.h"

static bool print_args(int argc, py_Ref argv) {
    py_ObjectRef tuple_item;
    py_i64 long_item;
    py_TValue* args = py_tuple_data(argv);
    int tuple_len = py_tuple_len(argv);
    printf("tuple_len: %d", tuple_len);
    PY_CHECK_ARGC(1); // single arg: `*args` tuple
    PY_CHECK_ARG_TYPE(0, tp_tuple); // check that it is
    for(int i = 0; i < tuple_len; i++) {
        tuple_item = py_tuple_getitem(py_arg(0), i);
        long_item = py_toint(tuple_item);
        printf("%d: %d", i, (int)long_item);
    }
    py_newnone(py_retval());
    return true;
}

bool demo_module_initialize(void) {
    py_GlobalRef mod = py_newmodule("demo");
    py_bind(mod, "print_args(*args)", print_args);
}

