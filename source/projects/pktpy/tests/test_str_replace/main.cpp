#include <iostream>
#include <string>

// build: g++ -o main main.cpp

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int main()
{
    std::cout << "testing basic replace" << std::endl;
    std::string string1("hello $name");
    replace(string1, "$name", "Somename");
    assert(string1.compare("hello Somename") == 0);
    std::cout << "hello $name -> " << string1 << std::endl;

    std::cout << std::endl;

    std::cout << "testing replace with empty string" << std::endl;
    std::string string2("hello \\$name");
    replace(string2, "\\", "");
    assert(string2.compare("hello $name") == 0);
    std::cout << "hello \\$name -> " << string2 << std::endl;
}