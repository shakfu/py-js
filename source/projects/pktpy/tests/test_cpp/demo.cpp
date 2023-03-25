#include "pocketpy.h"
#define PK_ENABLE_FILEIO 1

using namespace pkpy;


std::string read_file(std::string_view path) {
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);
    
    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}


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


    // test read_file()
    // std::string code = read_file("test.py");
    // // std::cout << code << std::endl;
    // vm->exec(code, "main.py", EXEC_MODE);
    // vm->exec("print(test())", "main.py", EXEC_MODE);


    std::ifstream t("test.py");
    std::stringstream buffer;
    buffer << t.rdbuf();
    // std::string code = read_file("test.py");
    // std::cout << code << std::endl;
    vm->exec(buffer.str(), "main.py", EXEC_MODE);
    vm->exec("print(test())", "main.py", EXEC_MODE);


    // test import
    // vm->exec("import test", "main.py", EXEC_MODE);
    // vm->exec("print(test.test())", "main.py", EXEC_MODE);

    // Eval the sum of the list
    // PyVar t_result = vm->exec("test.test()", "<eval>", EVAL_MODE);
    // std::cout << py_cast<Str>(vm, t_result);

    // std::cout << CAST(Str, s);  // abc

    // exception
    // std::cout << CAST(Str, result);

    // not working
    // char *str = CAST(Str, result);
    // std::cout << str;

    return 0;
}
