#include <iostream>

#include <giomm/init.h>

#include "command/runner.hh"
#include "input/handler.hh"
#include "parser/error.hh"
#include "parser/parser.hh"
#include "parser/types.hh"
#include "print.hh"


auto
main(int /* argc */, char ** /* argv */) -> int
{
    cmd::fill_binary_path_list();
    std::setlocale(LC_CTYPE, "");
    Gio::init();

    /* default, will change in the future */
    // output::Handler output { &std::cerr };
    input::Handler input { &std::cin };
    std::string    str;

    std::shared_ptr<parser::TokenGroup> tokens;

    while (!input.should_exit())
    {
        input.read(str);

        if (input.should_exit()) break;
        if (utils::str::is_empty(str)) continue;

        tokens = parser::parse(str);

        if (auto err { tokens->verify_syntax() }; err)
            io::println("{}", err->get_message());
        io::println("{}", Json::to_string(tokens->to_json()));
    }

    return 0;
}
