#include "pocketpy.h"
#include "stdio.h"

using namespace pkpy;

class PktpyInterpreter : public VM { };

int main() {
    PktpyInterpreter *py = new PktpyInterpreter();
    PyObject* result = py->exec("1+1", "<eval>", EVAL_MODE);
    if (is_int(result)) {
        int int_result = py_cast<int>(py, result);
        printf("result: %d\n", int_result);
    }
}