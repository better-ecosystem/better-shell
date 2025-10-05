#include <iostream>

#include <giomm/init.h>

#include "formatters.hh"
#include "input/handler.hh"
#include "parser/tokenizer.hh"
#include "print.hh"
#include "utils.hh"


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


        parser::Tokens tokens;

        try
        {
            tokens = parser::Tokens::tokenize(str);
            io::println("{}", Json::to_string(parser::Tokens::to_json(tokens)));
        }
        catch (const std::exception &e)
        {
            std::cerr << "parsing error: " << e.what() << '\n';
        }
    }

    return 0;
}
