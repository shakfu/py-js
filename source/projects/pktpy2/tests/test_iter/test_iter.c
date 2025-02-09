#include "pocketpy.h"
#include <stdio.h>


// add macros
#define PY_MAX_ELEMS 1024
#define ITER_SUCCESS 1
#define ITER_STOP 0
#define ITER_FAILURE (-1)
#define py_checklist(self) py_checktype(self, tp_list)
#define py_checktuple(self) py_checktype(self, tp_tuple)



static bool int_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);
    return true;
}


int pktpy2_handle_list_output(py_GlobalRef plist)
{
    if (plist == NULL) {
        goto error;
    }

    if (py_checklist(plist)) {
        int i = 0;

        int seq_size = py_list_len(plist);
        printf("seq_size: %d\n", seq_size);

        if (seq_size == 0) {
            printf("cannot process py list of length 0\n");
            goto error;
        }

        bool ok = py_iter(plist);
        if (!ok) {
            goto error;
        }

        py_GlobalRef iter = py_retval();
        py_setreg(0, iter);

        while (true) {
            int res = py_next(py_getreg(0));
            if (res == ITER_FAILURE) {
                printf("pktpy2_handle_list_output iter failed\n");
                goto error;
            }

            if (res == ITER_STOP)
                break;

            py_GlobalRef item = py_retval();


            if (py_isint(item)) {
                long long_item = py_toint(item);
                printf("%d long: %ld\n", i, long_item);
                i++;
            }

            if (py_isfloat(item)) {
                double float_item = py_isfloat(item);
                printf("%d float: %f\n", i, float_item);
                i++;
            }

            if (py_isstr(item)) {
                const char* unicode_item = py_tostr(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                printf("%d unicode: %s\n", i, unicode_item);
                i++;
            }
        }

        printf("end iter op: %d\n", i);

    }

    return 0;

error:
    printf("pktpy2_handle_list_output failed\n");
    return -1;
}



int main() {
    // Initialize pocketpy
    py_initialize();

    // Hello world!
    bool ok = py_exec("print('Hello world!')", "<string>", EXEC_MODE, NULL);
    if(!ok) goto __ERROR;

    // Create a list: [1, 2, 3]
    py_Ref r0 = py_getreg(0);
    py_newlistn(r0, 3);
    py_newint(py_list_getitem(r0, 0), 1);
    py_newint(py_list_getitem(r0, 1), 2);
    py_newint(py_list_getitem(r0, 2), 3);

    // iterate over list
    pktpy2_handle_list_output(r0);

    // Eval the sum of the list
    py_Ref f_sum = py_getbuiltin(py_name("sum"));
    py_push(f_sum);
    py_pushnil();
    py_push(r0);
    ok = py_vectorcall(1, 0);
    if(!ok) goto __ERROR;

    printf("Sum of the list: %d\n", (int)py_toint(py_retval()));  // 6

    // Bind native `int_add` as a global variable
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // Call `add` in python
    ok = py_exec("add(3, 7)", "<string>", EVAL_MODE, NULL);
    if(!ok) goto __ERROR;

    py_i64 res = py_toint(py_retval());
    printf("Sum of 2 variables: %d\n", (int)res);  // 10

    py_finalize();
    return 0;

__ERROR:
    py_printexc();
    py_finalize();
    return 1;
}
