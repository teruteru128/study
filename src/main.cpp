#include <iostream>

extern "C"
{
    int hiho(int, char **);
}

int main(int argc, char **argv)
{
    std::cout << "Hello, world!";
    std::cout << std::endl;
    return hiho(argc, argv);
}
