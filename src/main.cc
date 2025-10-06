#include <iostream>

#include <giomm/init.h>

#include "command/runner.hh"
#include "input/handler.hh"
#include "output/handler.hh"
#include "parser/error.hh"
#include "parser/parser.hh"
#include "parser/types.hh"
#include "print.hh"


auto
main(int /* argc */, char ** /* argv */) -> int
{
    // cmd::fill_binary_path_list();
    std::setlocale(LC_CTYPE, "");
    Gio::init();

    /* default, will change in the future */
    // output::Handler output { &std::cerr };
    input::Handler input { &std::cin };
    std::string    str;

    while (!input.should_exit())
    {
        size_t len { input.read(str) };

        if (input.should_exit()) break;
        if (len == 0) continue;

        parser::TokenGroup tokens { parser::parse(str) };

        auto err { tokens.verify_syntax() };
        if (!err)
            io::println("{}", tokens.to_json().toStyledString());
        else
            io::println("{}", err->get_message());
    }

    return 0;
}
