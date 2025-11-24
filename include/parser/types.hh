#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../error.hh"

namespace Json { class Value; }


namespace parser
{
    enum class TokenType : std::uint8_t
    {
        /* always occupy the first token of a TokenGroup */
        COMMAND,

        /* any token that are not of other token type
         * that sits in front of a command token */
        ARGUMENT,

        /* a token that starts with a dash (-) or a double-dash (--) */
        FLAG,

        /* any token that are not of other token type
         * that sits in front of a FLAG or ARGUMENT token,
         * or a string that is in front of a '=' after a FLAG or ARGUMENT token
        */
        PARAMETER,

        /* special character or string */
        OPERATOR,
        UNKNOWN,

        /* '{}'
         * a bracket that encloses a SUB_CONTENT */
        SUB_BRACKET,

        /* the content of a substitution */
        SUB_CONTENT,

        /* '/"
         * a quote (either single or double) that encloses a STRING_CONTENT */
        STRING_QUOTE,

        /* the content of a string */
        STRING_CONTENT,

        /* '{{}}'
         * a bracket that encloses an ARITHMETIC_EXPRESSION */
        ARITHMETIC_BRACKET,

        /* an arithmetic expression (1 + 1, 3^4, ...) */
        ARITHMETIC_EXPRESSION,

        /* '()'
         * a bracket that encloses a CONDITIONAL_EXPRESSION */
        CONDITIONAL_BRACKET,


        /* the content contained inside a CONDITIONAL_BRACKET */
        CONDITIONAL_CONTENT,

        /* a conditional/boolean expression that will get evaluated
         * !true || !value */
        CONDITIONAL_EXPRESSION,
    };


    enum class OperatorType : std::uint8_t
    {
        PIPE,
    };


    constexpr auto
    TokenType_to_string(TokenType t) noexcept -> std::string_view
    {
        switch (t)
        {
        case TokenType::COMMAND:            return "Type::COMMAND";
        case TokenType::ARGUMENT:           return "Type::ARGUMENT";
        case TokenType::FLAG:               return "Type::FLAG";
        case TokenType::PARAMETER:          return "Type::PARAMETER";
        case TokenType::OPERATOR:           return "Type::OPERATOR";
        case TokenType::UNKNOWN:            return "Type::UNKNOWN";
        case TokenType::SUB_BRACKET:        return "Type::SUB_BRACKET";
        case TokenType::SUB_CONTENT:        return "Type::SUB_CONTENT";
        case TokenType::STRING_QUOTE:       return "Type::STRING_QUOTE";
        case TokenType::STRING_CONTENT:     return "Type::STRING_CONTENT";
        case TokenType::ARITHMETIC_BRACKET: return "Type::ARITHMETIC_BRACKET";
        case TokenType::ARITHMETIC_EXPRESSION:
            return "Type::ARITHMETIC_EXPRESSION";

        case TokenType::CONDITIONAL_BRACKET: return "Type::CONDITIONAL_BRACKET";
        case TokenType::CONDITIONAL_CONTENT: return "Type::CONDITIONAL_CONTENT";
        case TokenType::CONDITIONAL_EXPRESSION:
            return "Type::CONDITIONAL_EXPRESSION";
        };
        return "Type::UNKNOWN";
    }


    constexpr auto
    OperatorType_to_string(OperatorType t) noexcept -> std::string_view
    {
        switch (t)
        {
        case OperatorType::PIPE: return "Operator::PIPE";
        }
        return "Operator::UNKNOWN";
    }


    struct Token;
    class Error;


    /**
     * a container for @e Token
     */
    struct TokenGroup
    {
        std::vector<Token> tokens;
        std::string        raw;


        /**
         * the place where the token got its input from, will be "stdin" if
         * input comes from std::cin
         */
        std::string source;


        /**
         * used to resolve errors
         */
        std::weak_ptr<TokenGroup> parent;


        TokenGroup(std::string raw, const std::shared_ptr<TokenGroup> &parent);


        /**
         * verify the "correctness" of the tokens
         * --------------------------------------
         *
         * the function basically checks for missing
         * closing/opening bracket/quotes
         *
         * returns a parser::Error if theres an error
         *
         * @warning the function mutates member @e tokens
         */
        [[nodiscard]]
        auto verify_syntax(bool conditional = false)
            -> std::optional<::error::Info>;


        /**
         * get the collected "syntax-highlighted" version of each token
         */
        [[nodiscard]]
        auto get_highlighted() const -> std::string;


        /**
         * get the top-most parent of this token group
         */
        [[nodiscard]]
        auto get_toplevel() const -> const TokenGroup *;


        /**
         * transform the tokens into a json object
         */
        [[nodiscard]]
        auto to_json() const -> Json::Value;


        /**
         * adds a token to the @e tokens list
         */
        template <typename... T_Args>
        void
        add_token(T_Args &&...args)
        {
            tokens.emplace_back(std::forward<T_Args>(args)...);
        }
    };

    using shared_tokens = std::shared_ptr<TokenGroup>;


    struct Token
    {
        TokenType type;


        /**
         * stores the first byte of @e data 's index from the parsed string
         */
        std::size_t index;


        /**
         * contains the raw text of the token or another token group
         * ----------------------------------
         *
         * the text will be '{' or '}' on @e type SUB_BRACKET,
         * " on @e type STRING_QUOTE,
         * or just a raw text on other types.
         */
        std::variant<std::string, shared_tokens> data;


        /**
         * stores the kind of operator if @e type is OPERATOR
         */
        std::optional<OperatorType> operator_type;


        Token() = default;
        Token(TokenType type, std::size_t idx, std::string data);
        Token(TokenType type, std::size_t idx, const shared_tokens &data);


        [[nodiscard]]
        auto operator==(const Token &other) const -> bool;


        /**
         * get the "syntax-highlighted" version of @e text
         * -----------------------------------------------
         *
         * the function will recursively run this function on every other Token
         * that the substitution contains when @e type is SUB_CONTENT
         */
        [[nodiscard]]
        auto get_highlighted() const -> std::string;


        /**
         * a wrapper for std::get_if ran on @e data
         */
        template <typename Tp>
        [[nodiscard]]
        auto
        get_data() const -> const Tp *
        {
            return std::get_if<Tp>(&this->data);
        }
    };
}
