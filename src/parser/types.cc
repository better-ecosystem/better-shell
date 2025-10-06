#include <stack>

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

            const std::string TEXT { front.get_data<std::string>() };

            if (!cmd::BINARY_PATH_LIST.contains(TEXT)
                && !cmd::built_in::COMMANDS.contains(TEXT))
            {
                return Error { tokens, ErrorType::PARSER_INVALID_COMMAND,
                               "command {} doesn't exist", 0, TEXT };
            }

            return std::nullopt;
        }


        [[nodiscard]]
        auto
        valid_string_quote(char ch) -> bool
        {
            return ch == '\'' || ch == '"' || ch == '`';
        }


        [[nodiscard]]
        auto
        valid_substitution_bracket(char ch) -> bool
        {
            return ch == '{' || ch == '}';
        }


        auto
        check_string_quote_token(
            const TokenGroup                    &tokens,
            std::stack<std::pair<char, size_t>> &quote_stack,
            size_t                               idx) -> std::optional<Error>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::STRING_QUOTE)
            {
                if (!token.attribute || !valid_string_quote(*token.attribute))
                    return Error {
                        tokens, ErrorType::PARSER_CORRUPTED_TOKEN_ATTRIBUTE,
                        "string-quote token has no valid attribute ({})", idx,
                        *token.attribute
                    };

                const char quote_char { *token.attribute };

                if (!quote_stack.empty()
                    && quote_stack.top().first == quote_char)
                    quote_stack.pop();
                else
                    quote_stack.emplace(quote_char, idx);
            }
            return std::nullopt;
        }


        auto
        check_substitution_bracket_token(const TokenGroup   &tokens,
                                         std::stack<size_t> &bracket_stack,
                                         size_t idx) -> std::optional<Error>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::SUB_BRACKET)
            {
                if (!token.attribute
                    || !valid_substitution_bracket(*token.attribute))
                    return Error { tokens,
                                   ErrorType::PARSER_CORRUPTED_TOKEN_ATTRIBUTE,
                                   "substitution-bracket token has no valid "
                                   "attribute ({})",
                                   idx, *token.attribute };

                if (*token.attribute == BracketKind::OPEN)
                    bracket_stack.push(idx);
                else if (*token.attribute == BracketKind::CLOSE)
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
    }


    auto
    TokenGroup::verify_syntax() -> std::optional<Error>
    {
        using enum TokenType;

        if (!verify_command(*this, this->tokens.front())) return std::nullopt;

        std::stack<std::pair<char, size_t>> quote_stack;
        std::stack<size_t>                  bracket_stack;

        for (size_t i { 1 }; i < this->tokens.size(); i++)
        {
            auto res { check_string_quote_token(*this, quote_stack, i) };
            if (res) return res;

            res = check_substitution_bracket_token(*this, bracket_stack, i);
            if (res) return res;
        }

        if (!bracket_stack.empty())
            return Error { *this, ErrorType::PARSER_UNCLOSED_BRACKET,
                           "unclosed bracket", bracket_stack.top() };

        if (!quote_stack.empty())
            return Error { *this, ErrorType::PARSER_UNCLOSED_QUOTE,
                           "unclosed quote", quote_stack.top().second };

        return std::nullopt;
    }


    auto
    TokenGroup::get_highlighted() -> std::string
    {
        std::string result;
        bool        first { true };

        for (auto &token : this->tokens)
        {
            if (!first) result += " ";

            result += token.get_highlighted() + " ";
            first   = false;
        }

        return result;
    }


    auto
    TokenGroup::to_json() const -> Json::Value
    {
        Json::Value root { Json::objectValue };

        root["raw"]    = this->raw;
        root["tokens"] = Json::arrayValue;

        for (const auto &token : this->tokens)
        {
            Json::Value j_token { Json::objectValue };
            j_token["type"] = Json::String { TokenType_to_string(token.type) };

            j_token["data"] = token.type == TokenType::SUB_CONTENT
                                ? token.get_data<TokenGroup>().to_json()
                                : token.get_data<std::string>();
            j_token["attribute"]
                = token.attribute ? *token.attribute : Json::nullValue;

            root["tokens"].append(j_token);
        }

        return root;
    }


    auto
    Token::get_highlighted() -> std::string
    {
        return "";
    }


    Token::~Token() {}
}
