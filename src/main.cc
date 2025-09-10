#include <iostream>
#include "input/input.hh"
#include "print.hh"


auto
main( int, char ** ) -> int
{
    input::Handler handler { &std::cin };
    std::string str;

    while (handler.read(str)) {
        if (handler.should_exit()) break;
        io::println("{}", str);
    }

    return 0;
}