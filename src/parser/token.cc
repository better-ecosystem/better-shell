#include <glibmm.h>
#include <giomm.h>
#include "parser/token.hh"

using namespace std::string_literals;
using parser::Token;


namespace
{
    [[nodiscard]]
    auto
    string_to_argv( const std::string &p_str ) -> std::vector<std::string>
    {
        std::istringstream iss { p_str };
        std::vector<std::string> argv;
        argv.reserve(std::ranges::count(p_str, ' ') + 1);
        for (std::string arg; std::getline(iss, arg, ' '); argv.push_back(arg));
        return argv;
    }


    [[nodiscard]]
    auto
    find_last_close_bracket( std::string_view p_str, size_t p_start ) -> size_t
    {
        size_t bracket_nest {0};

        for (size_t i = p_start; i < p_str.size(); ++i) {
            if (p_str[i] == '{') {
                bracket_nest++;
                continue;
            }

            if (p_str[i] == '}') {
                if (bracket_nest == 0)
                    throw std::invalid_argument("Unmatched closing bracket");
                bracket_nest--;
                if (bracket_nest == 0) return i;
            }
        }

        throw std::invalid_argument("No matching closing bracket found");
    }


    /**
     * @brief Parse the deepest part of a substitution flow,
     *        replacing the string with the result of the command.
     */
    void
    parse_substitution( std::string &p_str, size_t p_start )
    {
        size_t first_close { p_start };
        size_t last_open   { p_start };

        while (first_close < p_str.length() && p_str[first_close] != '}')
            first_close++;

        if (first_close == p_str.length())
            throw std::invalid_argument("Substitution not closed.");

        last_open = first_close;
        /* This is guaranteed to find an open bracket */
        while (p_str[last_open] != '$'
            || p_str[last_open + 1] != '{') last_open--;

        /* Now last_open points to "${" and first_close points to "}" */
        const std::string cmd { p_str.substr(last_open + 2,
                                             first_close - (last_open + 2)) };

        using Flags = Gio::Subprocess::Flags;
        auto proc { Gio::Subprocess::create(string_to_argv(cmd),
                    Flags::STDOUT_PIPE | Flags::STDERR_SILENCE) };

        auto stdout_stream = proc->get_stdout_pipe();
        Glib::RefPtr<Gio::DataInputStream> data_stream =
            Gio::DataInputStream::create(stdout_stream);

        std::string output;
        while (true) {
            std::string line;
            data_stream->read_line(line);
            if (line.empty()) break;
            output += line;
            output += '\n';
        }

        proc->wait_check();
        if (!output.empty() && output.back() == '\n')
            output.pop_back();

        p_str.replace(last_open, (first_close - last_open) + 1, output);
    }


    [[nodiscard]]
    auto
    get_command( const std::string &p_str ) -> std::string
    {
        std::istringstream iss { p_str };
        std::string word;
        iss >> word;
        return word;
    }
}


auto
Token::tokenize( std::string &p_str ) -> std::vector<Token>
{
    const size_t LEN { p_str.length() };

    std::vector<Token> tokens;
    tokens.emplace_back(TokenType::COMMAND, get_command(p_str));

    for (size_t i { tokens.back().data.length() }; i < LEN; i++) {
        if (std::isspace(p_str[i]) != 0) continue;

        /* Handle substitution. */
        if (p_str[i] == '$' && i + 1 < p_str.size() && p_str[i + 1] == '{') {
            size_t end_idx = find_last_close_bracket(p_str, i + 1);
            std::string cp_str { p_str.substr(i, (end_idx - i) + 1) };

            while (cp_str.find("${") != std::string::npos)
                parse_substitution(cp_str, 0);
            tokens.emplace_back(TokenType::SUBSTITUTE, cp_str);
            i = end_idx;
            continue;
        }

        /* Handle flags */
        if (p_str[i] == '-' && (std::isspace(p_str[i - 1]) != 0)) {
            /* Handles long arg */
            if (i + 1 < LEN && p_str[i + 1] == '-') {
                size_t len { 2 };
                while (i + len < LEN && std::isspace(p_str[i + len]) != 0)
                    len++;
                tokens.emplace_back(TokenType::FLAG, p_str.substr(i, len));
                continue;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len = 1;
            while (i + len < LEN && std::isspace(p_str[i + len]) != 0
                                 && p_str[i + len] != '=') {
                tokens.emplace_back(TokenType::FLAG, "-"s + p_str[i + len]);
                len++;
            }
            i += len;
            continue;
        }
    }

    return tokens;
}