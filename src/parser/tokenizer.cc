#include <giomm.h>
#include <glibmm.h>

#include "parser/tokenizer.hh"
#include "utils.hh"

using namespace std::string_literals;
using parser::Tokens;


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
Tokens::tokenize(std::string &str) -> Tokens
{
    const size_t LEN { str.length() };

    Tokens tokens;
    tokens.raw = str;

    tokens->emplace_back(TokenType::COMMAND, get_command(str));
    auto *data { std::get_if<std::string>(&tokens->back().data) };

    for (size_t i { data->length() }; i < LEN; i++)
    {
        if (std::isspace(str[i]) != 0) continue;

        /* Handle substitution. */
        if (str[i] == '$' && i + 1 < str.size() && str[i + 1] == '{')
        {
            size_t      end_idx { find_last_close_bracket(str, i + 1) };
            std::string cp_str { str.substr(i + 2, (end_idx - i - 2)) };

            tokens->emplace_back(TokenType::SUBSTITUTE, tokenize(cp_str));
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
                tokens->emplace_back(TokenType::FLAG, str.substr(i, len));
                i += len - 1;
                continue;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len = 1;
            while (i + len < LEN && std::isspace(str[i + len]) == 0
                   && str[i + len] != '=')
            {
                tokens->emplace_back(TokenType::FLAG, "-"s + str[i + len]);
                len++;
            }
            i += len;
            continue;
        }
    }

    return tokens;
}


auto
Tokens::to_json(const Tokens &tokens) -> Json::Value
{
    Json::Value root { Json::objectValue };

    root["raw"]    = tokens.raw;
    root["tokens"] = Json::arrayValue;

    for (const auto &token : tokens.tokens)
    {
        Json::Value json_token { Json::objectValue };

        json_token["type"] = Json::String { token_type_to_string(token.type) };

        if (token.type != TokenType::SUBSTITUTE)
        {
            json_token["data"]
                = Json::String { *std::get_if<std::string>(&token.data) };
        }
        else
        {
            json_token["data"]
                = Tokens::to_json(*std::get_if<Tokens>(&token.data));
        }

        root["tokens"].append(json_token);
    }

    return root;
}


auto
Tokens::from_json(const Json::Value &json) -> Tokens
{
    Tokens tokens;

    tokens->reserve(json["tokens"].size());
    tokens.raw = json["raw"].asString();

    for (const auto &json_token : json["tokens"])
    {
        Token token;
        token.type = token_type_from_string(json_token["type"].asString());

        if (token.type == TokenType::SUBSTITUTE)
        {
            token.data = Tokens::from_json(json_token["data"]);
        }
        else
        {
            token.data = json_token["data"].asString();
        }

        tokens->emplace_back(token);
    }

    return tokens;
}


auto
Tokens::operator->() -> std::vector<Token> *
{
    return &tokens;
}


void
parser::get_executable()
{
    std::string path { utils::getenv(
        "PATH", "/usr/local/sbin:/usr/local/bin:/usr/bin") };

    std::istringstream iss { path };
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
