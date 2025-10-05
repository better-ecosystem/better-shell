#include <iostream>

#include <giomm/init.h>

#include "formatters.hh"
#include "input/handler.hh"
#include "parser/tokenizer.hh"
#include "print.hh"


auto
main(int /* argc */, char ** /* argv */) -> int
{
    Gio::init();

    input::Handler handler { &std::cin };
    std::string    str;

    while (handler.read(str) != 0U)
    {
        if (handler.should_exit()) break;

        std::vector<parser::Token> tokens;
        try
        {
            tokens = parser::Token::tokenize(str);
            io::println("{}", tokens);
        }
        catch (const std::exception &e)
        {
            std::cerr << "parsing error: " << e.what() << '\n';
        }
    }

    return 0;
}
