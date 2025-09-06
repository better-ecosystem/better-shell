#include <iostream>
#include "input/input.hh"


auto
main( int p_argc, char **p_argv ) -> int
{
    input::Handler handler { &std::cin };
    std::string str;

    while (handler.read(str)) {
        std::cout << str << std::endl;
        std::cerr << "$ ";
    }

    return 0;
}