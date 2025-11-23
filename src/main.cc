#include <iostream>

#include <giomm/init.h>

#include "arg_parser.hh"
#include "command/runner.hh"
#include "error.hh"
#include "input/handler.hh"
#include "parser/parser.hh"
#include "print.hh"


namespace
{
    constexpr std::string_view HELP_TEXT {
        "{3}Usage:{8} {4}{0}{8} <arg {5}{{param}}{8}> <flag {5}{{param}}{8}> "
        "<path>\n"
        "{1}\n"
        "\n"
        "  {3}Args:{8}\n"
        "      {4}help{8}     h                show this message\n"
        "      {4}version{8}  v                show version info\n"
        "\n"
        "  {3}Flags:{8}\n"
        "    {4}--command{8} -c {5}{{command}}{8}    run command then exit\n"
        "    {4}--config{8}  -C {5}{{path}}{8}       specify config path\n"
        "\n"
        "  {6}Parameter passed to the `command` flag\n"
        "    must be covered in a double quotation mark (\"){8}\n"
        "\n"
        "--------------------------------------------------\n"
        "\n"
        "  If the config path is not specified with the\n"
        "  `config` flag, {4}{2}{8} will search the config in\n"
        "  these places according to the specified order\n"
        "\n"
        "    1. $XDG_CONFIG_HOME/better/shell/config.bsh\n"
        "    2. $HOME/better/shell/config.bsh\n"
        "    3. $HOME/.better_shell.bsh\n"
        "    4. /etc/better/shell/config.bsh\n"
    };


    [[noreturn]]
    void
    print_help_message(std::string_view binary)
    {
        io::println(HELP_TEXT, binary, std::string(binary.length() + 44, '-'),
                    APP_NAME, ANSI_RGB_FG(0, 200, 255),
                    ANSI_RGB_FG(120, 220, 120), ANSI_RGB_FG(255, 200, 0),
                    ANSI_RGB_FG(255, 140, 64), ANSI_RGB_FG(255, 140, 255),
                    COLOR_RESET);
        std::exit(0);
    }


    [[noreturn]]
    void
    print_version_info()
    {
        io::println("{} {}", APP_NAME, APP_VERSION);
        std::exit(0);
    }


    [[nodiscard]]
    auto
    combine_argv(int &argc, char **&argv) -> std::string
    {
        std::ostringstream oss;

        for (int i { 0 }; i < argc; i++)
        {
            oss << argv[i];
            if (i < argc - 1) oss << ' ';
        }

        return oss.str();
    }


    template <typename... T_Args>
    [[nodiscard]]
    auto
    make_error_message(const std::string &combined_argv,
                       std::size_t        position,
                       std::size_t        length,
                       T_Args &&...args) -> std::string
    {
        auto info { error::Info(std::forward<T_Args>(args)...) };
        info.set_error_context("argv", combined_argv, position, length);
        return info.create_pretty_message();
    }


    [[nodiscard]]
    auto
    get_command_param(int                            &argc,
                      char                         **&argv,
                      const std::vector<std::string> &args,
                      ArgIndex                        start,
                      bool                           &err) -> std::string
    {
        const std::string  combined_argv { combine_argv(argc, argv) };
        const std::string &flag_arg { args[start.arg_idx] };

        const std::size_t flag_position { combined_argv.find(flag_arg) };
        error::assert(flag_position != std::string::npos,
                      std::format("flag \"{}\" is not found in argv \"{}\"",
                                  flag_arg, combined_argv));

        std::size_t first_quote { std::string::npos };

        if (std::size_t eq_idx { flag_arg.find('=') };
            eq_idx != std::string::npos)
        {
            std::string flag { flag_arg.substr(0, eq_idx) };

            first_quote = flag_position + eq_idx + 1;
            while (std::isspace(combined_argv[first_quote]) != 0) first_quote++;

            if (combined_argv[first_quote] != '"')
            {
                err = true;
                return make_error_message(
                    combined_argv, flag_position, flag.length(), "no parameter",
                    "No valid parameter passed to {}", flag);
            }

            std::size_t second_quote { combined_argv.find('"',
                                                          first_quote + 1) };

            if (second_quote != std::string::npos)
                return combined_argv.substr(first_quote + 1,
                                            second_quote - first_quote - 1);

            err = true;
            return make_error_message(
                combined_argv, first_quote,
                combined_argv.length() - first_quote, "unclosed quote",
                "No closing quote found for parameter passed to {}", flag);
        }

        std::size_t i { flag_position + flag_arg.length() };
        while (std::isspace(combined_argv[i]) != 0) i++;

        if (combined_argv[i] != '"')
        {
            err = true;
            return make_error_message(
                combined_argv, flag_position, flag_arg.length(), "no parameter",
                "No valid parameter passed to {}", flag_arg);
        }

        first_quote = i;
        std::size_t second_quote { combined_argv.find('"', i + 1) };
        if (second_quote == std::string::npos)
        {
            err = true;
            return make_error_message(
                combined_argv, first_quote,
                combined_argv.length() - first_quote, "unclosed quote",
                "No closing quote found for parameter passed to {}", flag_arg);
        }

        return combined_argv.substr(first_quote + 1,
                                    second_quote - first_quote - 1);
    }
}


auto
main(int argc, char **argv) -> int
{
    cmd::fill_binary_path_list();
    std::setlocale(LC_ALL, "");
    Gio::init();

    ArgParser arg_parser { argc, argv };
    if (arg_parser.is_arg("help", 'h', false)) print_help_message(*argv);
    if (arg_parser.is_arg("version", 'v', false)) print_version_info();

    auto command_flag { arg_parser.is_flag("command", 'c', true) };

    std::unique_ptr<std::istream> stream;

    if (!command_flag)
        stream = std::make_unique<std::istream>(std::cin.rdbuf());
    else
    {
        const auto &args { arg_parser.get_args() };
        bool        err { false };
        std::string param { get_command_param(argc, argv, args, *command_flag,
                                              err) };

        if (err)
        {
            io::println("{}", param);
            return EINVAL;
        }

        stream = std::make_unique<std::istringstream>(param);
    }

    input::Handler input { stream.get() };
    std::string    text;
    std::string    source { command_flag ? "argv" : "stdin" };

    while (!input.should_exit())
    {
        input.read(text);

        if (!command_flag)
        {
            if (input.should_exit()) break;
            if (utils::str::is_empty(text)) continue;
        }

        auto tokens { parser::parse(source, text) };

        if (auto err { tokens->verify_syntax() })
            io::println("{}", err->create_pretty_message());

        io::println("{}", Json::to_string(tokens->to_json()));
    }

    return 0;
}
