#include <stack>
#include <utility>

#include "command/built_in.hh"
#include "command/runner.hh"
#include "parser/error.hh"
#include "parser/types.hh"

using namespace std::literals;


namespace parser
{
    namespace
    {
        [[nodiscard]]
        auto
        verify_command(TokenGroup &tokens, Token &front) -> std::optional<Error>
        {
            if (front.type != TokenType::COMMAND)
            {
                return Error { tokens,
                               ErrorType::PARSER_FIRST_TOKEN_IS_NOT_COMMAND,
                               "the first token is not a command", 0 };
            }

            const std::string *TEXT { front.get_data<std::string>() };

            if (TEXT->starts_with("./"))
            {
                std::filesystem::path path { TEXT->substr(2) };
                if (!std::filesystem::exists(path))
                {
                    return Error { tokens, ErrorType::PARSER_INVALID_COMMAND,
                                   "executable path '{}' doesn't exist", 0,
                                   *TEXT };
                }

                path = std::filesystem::canonical(path);
                if (!std::filesystem::is_regular_file(path))
                {
                    return Error { tokens, ErrorType::PARSER_INVALID_COMMAND,
                                   "path '{}' is not a file", 0, *TEXT };
                }

                if (access(path.c_str(), X_OK) != 0)
                {
                    return Error { tokens, ErrorType::PARSER_INVALID_COMMAND,
                                   "path '{}' is not an executable", 0, *TEXT };
                }

                return std::nullopt;
            }

            if (!cmd::BINARY_PATH_LIST.contains(*TEXT)
                && !cmd::built_in::COMMANDS.contains(*TEXT))
            {
                return Error { tokens, ErrorType::PARSER_INVALID_COMMAND,
                               "command '{}' doesn't exist", 0, *TEXT };
            }

            return std::nullopt;
        }


        [[nodiscard]]
        auto
        check_string_quote_token(TokenGroup &tokens,
                                 size_t     &quote_idx,
                                 size_t      idx) -> std::optional<Error>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::STRING_QUOTE)
            {
                if (quote_idx != std::string::npos)
                {
                    if (quote_idx + 1 < tokens.tokens.size()
                        && tokens.tokens[quote_idx + 1].type
                               != TokenType::STRING_CONTENT)
                    {
                        return Error { tokens, ErrorType::PARSER_EMPTY_STRING,
                                       "string is empty", quote_idx };
                    }
                    quote_idx = std::string::npos;
                }
                else
                    quote_idx = idx;
            }
            return std::nullopt;
        }


        [[nodiscard]]
        auto
        check_substitution_bracket_token(TokenGroup         &tokens,
                                         std::stack<size_t> &bracket_stack,
                                         size_t idx) -> std::optional<Error>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::SUB_BRACKET)
            {
                const std::string *text { token.get_data<std::string>() };
                const char         bracket { (*text)[0] };

                if (bracket == '{')
                {
                    bracket_stack.push(idx);

                    if (idx + 1 < tokens.tokens.size()
                        && tokens.tokens[idx + 1].type
                               != TokenType::SUB_CONTENT)
                    {
                        return Error { tokens,
                                       ErrorType::PARSER_EMPTY_SUBSTITUTION,
                                       "substitution has no content", idx };
                    }
                }
                else if (bracket == '}')
                {
                    if (!bracket_stack.empty())
                        bracket_stack.pop();
                    else
                    {
                        /* unexpected closing bracket */
                        return Error { tokens,
                                       ErrorType::PARSER_UNCLOSED_BRACKET,
                                       "unmatched closing bracket", idx };
                    }
                }
            }

            return std::nullopt;
        }


        [[nodiscard]]
        auto
        check_parameter_token(TokenGroup &tokens, size_t idx)
            -> std::optional<Error>
        {
            if (tokens.tokens[idx].type != TokenType::PARAMETER)
                return std::nullopt;

            const auto *text { tokens.tokens[idx].get_data<std::string>() };

            if (!text->empty()) return std::nullopt;

            return Error { tokens, ErrorType::PARSER_EMPTY_PARAM,
                           "parameter not given", idx };
        }
    }


    auto
    TokenGroup::verify_syntax() -> std::optional<Error>
    {
        using enum TokenType;

        auto err { verify_command(*this, this->tokens.front()) };
        if (err) return err;

        std::stack<size_t> bracket_stack;
        size_t             quote_idx { std::string::npos };

        for (size_t i { 1 }; i < this->tokens.size(); i++)
        {
            if (this->tokens[i].type == SUB_CONTENT)
            {
                auto res { this->tokens[i]
                               .get_data<shared_tokens>()
                               ->get()
                               ->verify_syntax() };
                if (res) return res;
            }

            auto res { check_parameter_token(*this, i) };
            if (res) return res;

            res = check_string_quote_token(*this, quote_idx, i);
            if (res) return res;

            res = check_substitution_bracket_token(*this, bracket_stack, i);
            if (res) return res;
        }

        if (!bracket_stack.empty())
            return Error { *this, ErrorType::PARSER_UNCLOSED_BRACKET,
                           "unclosed bracket", bracket_stack.top() };

        if (quote_idx != std::string::npos)
            return Error { *this, ErrorType::PARSER_UNCLOSED_QUOTE,
                           "unclosed quote", quote_idx };

        return std::nullopt;
    }


    auto
    TokenGroup::get_highlighted() const -> std::string
    {
        std::string result;
        bool        first { true };

        for (const auto &token : this->tokens)
        {
            if (!first) result += " ";

            result += token.get_highlighted() + " ";
            first   = false;
        }

        return result;
    }


    auto
    TokenGroup::to_json() -> Json::Value
    {
        Json::Value root { Json::objectValue };

        root["raw"]    = this->raw;
        root["tokens"] = Json::arrayValue;

        for (auto &token : this->tokens)
        {
            Json::Value j_token { Json::objectValue };
            j_token["type"] = Json::String { TokenType_to_string(token.type) };

            j_token["data"]
                = token.type == TokenType::SUB_CONTENT
                    ? token.get_data<shared_tokens>()->get()->to_json()
                    : *token.get_data<std::string>();
            j_token["index"] = token.index;

            root["tokens"].append(j_token);
        }

        return root;
    }


    auto
    TokenGroup::get_toplevel() const -> const TokenGroup *
    {
        const TokenGroup *current { this };
        while (auto parent_ptr { current->parent.lock() })
            current = parent_ptr.get();

        return current;
    }


    TokenGroup::TokenGroup(std::string raw, const shared_tokens &parent)
        : tokens({}), raw(std::move(raw)), parent(parent)
    {
    }


    Token::Token(TokenType type, size_t idx, std::string data)
        : type(type), index(idx), data(std::move(data))
    {
    }


    Token::Token(TokenType type, size_t idx, const shared_tokens &data)
        : type(type), index(idx), data(data), operator_type(OperatorType::NONE)
    {
    }


    auto
    Token::get_highlighted() const -> std::string
    {
        return "";
    }
}
