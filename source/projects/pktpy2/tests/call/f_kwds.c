#include "pocketpy.h"
#include <stdio.h>

/*

print_kwds(a=1, b=2)


*/


#define INPUT ""


static bool my_print_kw_dict_apply(py_Ref key, py_Ref value, void *ctx) {
  const char *sep = (const char *)ctx;
  if (!py_str(key)) return false;
  printf("%s", py_tostr(py_retval()));
  if (!py_str(value)) return false;
  printf("%s%s\n", sep, py_tostr(py_retval()));
  return true;
}

static bool print_kwds(int argc, py_Ref argv) {
  PY_CHECK_ARGC(2);
  PY_CHECK_ARG_TYPE(0, tp_str);
  PY_CHECK_ARG_TYPE(1, tp_dict);
  const char *sep = py_tostr(py_arg(0));
  bool ok = py_dict_apply(py_arg(1), my_print_kw_dict_apply, (void *)sep);
  if (!ok) return false;
  py_newnone(py_retval());
  return true;
}

int main() {
    py_initialize();

    py_GlobalRef __main__ = py_getmodule("__main__");
    py_bind(__main__, "print_kwds(**kwds)", print_kwds);

    if (!py_exec(INPUT, "main.py", EXEC_MODE, NULL)) {
        py_printexc();
    }

    py_finalize();
    return 0;
}