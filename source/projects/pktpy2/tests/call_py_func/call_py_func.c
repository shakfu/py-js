#include "pocketpy.h"
#include <stdio.h>

#define INPUT ""

/*
def multiply(x, y):
    return x * y


class Multiplier:
    def __init__(self, x):
        self.x = x

    def multiply(self, y):
        return self.x * y
*/

int main() {
  py_initialize();

  if (!py_exec(INPUT, "main.py", EXEC_MODE, NULL)) {
    py_printexc();
    goto finalize;
  }

  int x = 10;
  int y = 3;

  py_GlobalRef __main__ = py_getmodule("__main__");

  // multiply(x, y)
  if (!py_getattr(__main__, py_name("multiply"))) {
    py_printexc();
    goto finalize;
  }

  py_push(py_retval());       // callable
  py_pushnil();               // self or nil
  py_newint(py_pushtmp(), x); // arg1
  py_newint(py_pushtmp(), y); // arg2

  if (!py_vectorcall(2, 0)) {
    py_printexc();
    goto finalize;
  }

  if (py_isint(py_retval())) {
    int res = py_toint(py_retval());
    printf("multiply(10, 3): %d\n", res);
  }

  // Multiplier(x).multiply(y)
  py_newint(py_r0(), x);
  py_newint(py_r1(), y);
  if (!py_smarteval("Multiplier(_0).multiply(_1)", NULL, py_r0(), py_r1())) {
    py_printexc();
    goto finalize;
  }

  if (py_isint(py_retval())) {
    int res = py_toint(py_retval());
    printf("Multiplier(10).multiply(3): %d\n", res);
  }

finalize:
  py_finalize();
  return 0;
}
