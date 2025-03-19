#include "pocketpy.h"
#include <stdio.h>

#define INPUT ""

/*
print("==> my_print(*args, sep=', ')")
my_print(1, '2', 3.0, True, [None, ('a', 'b')])

a = [i for i in range(5)]
my_print(*a, sep=' | ')

print("==> my_print_kw(sep='=', **kwargs)")
my_print_kw(a=1, b='2', c=3.0, d=True)
print()
my_print_kw(a=1, b='2', c=3.0, d=True, sep=': ')

*/

static bool my_print(int argc, py_Ref argv) {
  PY_CHECK_ARGC(2);
  PY_CHECK_ARG_TYPE(0, tp_tuple);
  PY_CHECK_ARG_TYPE(1, tp_str);

  int length = py_tuple_len(py_arg(0));
  const char *sep = py_tostr(py_arg(1));

  for (int i = 0; i < length; i++) {
    py_Ref item = py_tuple_getitem(py_arg(0), i);
    if (!py_str(item)) {
      return false;
    }
    printf("%s", py_tostr(py_retval()));
    if (i < length - 1)
      printf("%s", sep);
  }
  printf("\n");
  py_newnone(py_retval());
  return true;
}

static bool my_print_kw_dict_apply(py_Ref key, py_Ref value, void *ctx) {
  const char *sep = (const char *)ctx;
  if (!py_str(key)) return false;
  printf("%s", py_tostr(py_retval()));
  if (!py_str(value)) return false;
  printf("%s%s\n", sep, py_tostr(py_retval()));
  return true;
}

static bool my_print_kw(int argc, py_Ref argv) {
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
  py_bind(__main__, "my_print(*args, sep=', ')", my_print);
  py_bind(__main__, "my_print_kw(sep='=', **kwargs)", my_print_kw);

  if (!py_exec(INPUT, "main.py", EXEC_MODE, NULL)) {
    py_printexc();
  }

  py_finalize();
  return 0;
}
