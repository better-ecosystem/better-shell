#include <iostream>
#include <giomm/init.h>
#include "input/handler.hh"
#include "print.hh"


auto
main( int /* p_argc */, char ** /* p_argv */ ) -> int
{
    Gio::init();

    input::Handler handler { &std::cin };
    std::string str;

    while (handler.read(str) != 0U) {
        if (handler.should_exit()) break;
        io::println("{}", str);
    }

    return 0;
}