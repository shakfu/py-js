#include "pocketpy.h"
#include <stdio.h>

int main() {
  py_initialize();

  // string
  py_newstr(py_r0(), "hello world");
  const char *str_ = py_tostr(py_r0());
  printf("str: %s\n", str_);

  // int
  py_newint(py_r1(), 10);
  int int_ = py_toint(py_r1());
  printf("int: %d\n", int_);

  // float
  py_newfloat(py_r2(), 10.5);
  float float_ = py_tofloat(py_r2());
  printf("float: %f\n", float_);

  // tuple (r3)
  py_Ref p = py_newtuple(py_r3(), 3);
  py_assign(py_offset(p, 0), py_r0());
  py_assign(py_offset(p, 1), py_r1());
  py_assign(py_offset(p, 2), py_r2());
  if (!py_repr(py_r3())) {
    py_printexc();
    goto finalize;
  }
  printf("tuple: %s\n", py_tostr(py_retval()));

  // list (r4)
  py_newlist(py_r4());
  py_list_append(py_r4(), py_r0());
  py_list_append(py_r4(), py_r1());
  py_list_append(py_r4(), py_r2());
  if (!py_repr(py_r4())) {
    py_printexc();
    goto finalize;
  }
  printf("list: %s\n", py_tostr(py_retval()));

  // dict (r5)
  py_newdict(py_r5());
  py_dict_setitem_by_str(py_r5(), "str", py_r0());
  py_dict_setitem_by_str(py_r5(), "int", py_r1());
  py_dict_setitem_by_str(py_r5(), "float", py_r2());
  py_dict_setitem_by_str(py_r5(), "tuple", py_r3());
  py_dict_setitem_by_str(py_r5(), "list", py_r4());
  if (!py_repr(py_r5())) {
    py_printexc();
    goto finalize;
  }
  printf("dict: %s\n", py_tostr(py_retval()));

finalize:
  py_finalize();
  return 0;
}
