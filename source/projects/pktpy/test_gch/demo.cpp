#include "pocketpy.hpp"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM(true);
    
    // Hello world!
    vm->exec("print('Hello world!')", "main.py", EXEC_MODE);

    // Create a list
    vm->exec("a = [1, 2, 3]", "main.py", EXEC_MODE);

    // Eval the sum of the list
    PyVar result = vm->exec("sum(a)", "<eval>", EVAL_MODE);
    std::cout << py_cast<i64>(vm, result);   // 6
    return 0;
}