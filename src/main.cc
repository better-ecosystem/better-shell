#include <iostream>
#include "input/input.hh"


auto
main( int p_argc, char **p_argv ) -> int
{
    input::Handler handler {};
    std::string str;

    while (handler.read(std::cin, str)) {
        std::cout << str << std::endl;
    }

    return 0;
}