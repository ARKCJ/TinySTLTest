#include <iostream>
#include <string>
#include "Any.h"


int main()
{
    using namespace test;
    Any any;
    any = 1;
    std::cout << any.Cast<int>() << std::endl;
    any = "123";
    std::cout << any.Cast<const char*>() << std::endl;
    any = 1.45;
    std::cout << any.Cast<double>() << std::endl;
    any = std::string("my_string");
    std::cout << any.Cast<std::string>() << std::endl;
    std::cout << any.Type().name() << std::endl;
}