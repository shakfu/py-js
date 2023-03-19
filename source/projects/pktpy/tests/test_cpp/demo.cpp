#include "pocketpy.h"

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

    // std::cout << py_cast<i64>(vm, result);   // 6
    std::cout << py_cast<i64>(vm, result);

    PyVar s = VAR("abc");
    Str st = CAST(Str, s); // Str is a subclass of std::string

    printf("\nas c string: %s\n", st.c_str());

    std::cout << st;

    // std::cout << CAST(Str, s);  // abc

    // exception
    // std::cout << CAST(Str, result);

    // not working
    // char *str = CAST(Str, result);
    // std::cout << str;

    return 0;
}
