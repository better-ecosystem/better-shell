#include "arg_parser.hh"

using namespace std::literals;


ArgParser::ArgParser(int &argc, char **&argv) : m_args(argv + 1, argv + argc) {}


auto
ArgParser::is_flag(std::string_view long_flag,
                   char             short_flag,
                   bool             accept_param) -> std::optional<ArgIndex>
{
    for (size_t i { 0 }; i < m_args.size(); i++)
    {
        std::string &arg { m_args[i] };

        if (arg.starts_with("--"))
        {
            /* arg without the dashes */
            std::string_view trimmed { arg.data() + 2 };

            if (!trimmed.starts_with(long_flag)) continue;

            const size_t LONG_FLAG_LEN { long_flag.length() };

            if (trimmed.length() > LONG_FLAG_LEN
                && trimmed[LONG_FLAG_LEN] != '=')
                continue;

            if (!accept_param) m_args.erase(m_args.begin() + i);

            return { i };
        }

        if (arg.starts_with('-'))
            for (size_t j { 1 }; j < arg.length(); j++)
            {
                if (arg[j] == '=') break;
                if (arg[j] != short_flag) continue;
                if (!accept_param) arg.erase(j, 1);
                if (arg == "-") m_args.erase(m_args.begin() + i);

                return ArgIndex { i, j };
            }
    }

    return std::nullopt;
}


auto
ArgParser::is_arg(std::string_view long_arg, char short_arg, bool accept_param)
    -> std::optional<ArgIndex>
{
    if (m_args.size() >= 1
        && (m_args[0] == long_arg || m_args[0] == std::string { short_arg }))
    {
        if (!accept_param) m_args.erase(m_args.begin());
        return { 1 };
    }

    return std::nullopt;
}


auto
ArgParser::get_args() const -> const std::vector<std::string> &
{
    return m_args;
}
