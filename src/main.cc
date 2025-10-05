#include <iostream>

#include <giomm/init.h>

#include "formatters.hh"
#include "input/handler.hh"
#include "parser/tokenizer.hh"
#include "print.hh"


auto
main(int /* argc */, char ** /* argv */) -> int
{
    std::setlocale(LC_CTYPE, "");
    Gio::init();

    input::Handler handler { &std::cin };
    std::string    str;

    while (!handler.should_exit())
    {
        size_t len { handler.read(str) };

        if (handler.should_exit()) break;
        if (len == 0) continue;


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
