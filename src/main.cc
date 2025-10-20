#include <iostream>

#include <giomm/init.h>

#include "arg_parser.hh"
#include "command/runner.hh"
#include "input/handler.hh"
#include "parser/error.hh"
#include "parser/parser.hh"
#include "parser/types.hh"
#include "print.hh"


namespace
{
    [[noreturn]]
    void
    print_help_message(std::string_view binary)
    {
        io::println("Usage: {} <options {{params}}>", binary);
        io::println("{}\n", std::string(binary.length() + 26, '-'));

        io::println("  --help    -h                show this message");
        io::println("  --version -V                show version info");
        io::println("  --command -c {{command}}    run command then exit");
        io::println("  --config  -C {{path}}       specify config path");
        std::exit(0);
    }


    [[noreturn]]
    void
    print_version_info()
    {
        io::println("{} {}", APP_NAME, APP_VERSION);
        std::exit(0);
    }
}


auto
main(int argc, char **argv) -> int
{
    cmd::fill_binary_path_list();
    std::setlocale(LC_CTYPE, "");
    Gio::init();

    ArgParser arg_parser { argc, argv };
    if (arg_parser.is_flag("help", 'h', false)) print_help_message(*argv);
    if (arg_parser.is_flag("version", 'V', false)) print_version_info();

    auto command_flag { arg_parser.is_flag("command", 'c', false) };

    std::unique_ptr<std::istream> stream;

    if (!command_flag)
        stream = std::make_unique<std::istream>(std::cin.rdbuf());
    else
    {
        auto               args { arg_parser.get_args() };
        std::ostringstream oss;

        for (int i { command_flag->arg_idx }; (size_t)i < args.size(); i++)
        {
            if (i != command_flag->arg_idx) oss << ' ';
            oss << args[i];
        }

        stream = std::make_unique<std::istringstream>(oss.str());
    }

    input::Handler input { stream.get() };
    std::string    str;

    std::shared_ptr<parser::TokenGroup> tokens;
    while (!input.should_exit())
    {
        input.read(str);

        if (!command_flag)
        {
            if (input.should_exit()) break;
            if (utils::str::is_empty(str)) continue;
        }

        tokens = parser::parse(str);

        if (auto err { tokens->verify_syntax() }; err)
            io::println("{}", err->create_pretty_message());
        io::println("{}", Json::to_string(tokens->to_json()));
    }

    return 0;
}
