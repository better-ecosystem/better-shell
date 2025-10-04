#include <giomm.h>
#include <glibmm.h>

#include "parser/token.hh"

using namespace std::string_literals;
using parser::Token;


namespace
{
    [[nodiscard]]
    auto
    find_last_close_bracket(std::string_view str, size_t start) -> size_t
    {
        size_t bracket_nest { 0 };

        for (size_t i = start; i < str.size(); ++i)
        {
            if (str[i] == '{')
            {
                bracket_nest++;
                continue;
            }

            if (str[i] != '{')
                throw std::invalid_argument(
                    "start index doesn't point at an open curly braces");

            if (str[i] == '}')
            {
                if (bracket_nest == 0)
                    throw std::invalid_argument("Unmatched closing bracket");
                bracket_nest--;
                if (bracket_nest == 0) return i;
            }
        }

        throw std::invalid_argument("No matching closing bracket found");
    }


    [[nodiscard]]
    auto
    get_command(const std::string &str) -> std::string
    {
        std::istringstream iss { str };
        std::string        word;
        iss >> word;
        return word;
    }
}


auto
Token::tokenize(std::string &str) -> std::vector<Token>
{
    const size_t LEN { str.length() };

    std::vector<Token> tokens;
    tokens.emplace_back(TokenType::COMMAND, get_command(str));
    auto *data { std::get_if<std::string>(&tokens.back().data) };

    for (size_t i { data->length() }; i < LEN; i++)
    {
        if (std::isspace(str[i]) != 0) continue;

        /* Handle substitution. */
        if (str[i] == '$' && i + 1 < str.size() && str[i + 1] == '{')
        {
            size_t      end_idx { find_last_close_bracket(str, i + 1) };
            std::string cp_str { str.substr(i + 2, (end_idx - i - 2)) };
            tokens.emplace_back(TokenType::SUBSTITUTE, tokenize(cp_str));
            i = end_idx;
            continue;
        }

        /* Handle flags */
        if (str[i] == '-' && (std::isspace(str[i - 1]) != 0))
        {
            /* Handles long arg */
            if (i + 1 < LEN && str[i + 1] == '-')
            {
                size_t len { 2 };
                while (i + len < LEN && std::isspace(str[i + len]) == 0) len++;
                tokens.emplace_back(TokenType::FLAG, str.substr(i, len));
                i += len - 1;
                continue;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len = 1;
            while (i + len < LEN && std::isspace(str[i + len]) == 0
                   && str[i + len] != '=')
            {
                tokens.emplace_back(TokenType::FLAG, "-"s + str[i + len]);
                len++;
            }
            i += len;
            continue;
        }
    }

    return tokens;
}


void
parser::get_executable()
{
    const char *PATH { std::getenv("PATH") };
    if (PATH == nullptr) return;

    std::istringstream iss { PATH };
    for (std::string dir; std::getline(iss, dir, ':');)
    {
        if (dir.empty()) continue;

        std::filesystem::recursive_directory_iterator it { dir };
        for (const auto &entry : it)
        {
            if (!entry.is_regular_file()) continue;

            const auto &file { entry.path() };
            if (access(file.c_str(), X_OK) == 0)
                BINARIES[file.filename().string()] = file;
        }
    }
}
