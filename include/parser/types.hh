#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <json/value.h>


namespace parser
{
    enum class TokenType : uint8_t
    {
        /**
         * holds the command to be ran, must be the first
         * word in a scope
         */
        COMMAND,

        /**
         * the argument to a command is the direct string that comes after it
         * the string must not be a punctuation
         */
        ARGUMENT,

        /**
         * flags are strings that starts with dashes, flag can be either
         * a long flag (starts with 2 dashes '--'), or
         * a short flag (starts with 1 dash '-')
         */
        FLAG,

        /**
         * operators (+, -, ||, /, |, &, ...)
         * this includes commas, stars and such
         */
        OPERATOR,

        UNKNOWN,

        /* SUBSTITUTION TOKEN TYPE */

        /**
         * the bracket of the substitution, can either be OPEN or CLOSE,
         * on OPEN the text shall contain ${,
         * and on CLOSE it will only contain }
         */
        SUB_BRACKET,

        /**
         * the content of the substitution, which will contain another tokens
         */
        SUB_CONTENT,

        /* STRING TOKEN TYPE */

        /**
         * the quote of the string, can be ", or '
         */
        STRING_QUOTE,

        /**
         * the content of the string, not including the quotation marks.
         */
        STRING_CONTENT,
    };


    constexpr auto
    TokenType_to_string(TokenType t) noexcept -> std::string_view
    {
        switch (t)
        {
        case TokenType::COMMAND:        return "Type::COMMAND";
        case TokenType::ARGUMENT:       return "Type::ARGUMENT";
        case TokenType::FLAG:           return "Type::FLAG";
        case TokenType::OPERATOR:       return "Type::OPERATOR";
        case TokenType::UNKNOWN:        return "Type::UNKNOWN";
        case TokenType::SUB_BRACKET:    return "Type::SUB_BRACKET";
        case TokenType::SUB_CONTENT:    return "Type::SUB_CONTENT";
        case TokenType::STRING_QUOTE:   return "Type::STRING_QUOTE";
        case TokenType::STRING_CONTENT: return "Type::STRING_CONTENT";
        };
    }


    class BracketKind
    {
    public:
        static constexpr char OPEN  { '{' };
        static constexpr char CLOSE { '}' };
    };

    class FlagKind
    {
    public:
        static constexpr char SHORT { '-' };
        static constexpr char LONG  { '=' };
    };


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
        auto verify_syntax() -> std::optional<Error>;


        /**
         * get the collected "syntax-highlighted" version of each token
         */
        [[nodiscard]]
        auto get_highlighted() -> std::string;


        /**
         * transform the tokens into a json object
         */
        [[nodiscard]]
        auto to_json() const -> Json::Value;
    };


    struct Token
    {
        TokenType type;


        /**
         * stores the first byte of @e data 's index from the parsed string
         */
        size_t index;


        /**
         * contains the raw text of the token or another token group
         * ----------------------------------
         *
         * the text will be '${' or '}' on @e type SUB_BRACKET,
         * ''', '"', or '`' on @e type STRING_QUOTE,
         * or just a raw text on other types.
         */
        std::variant<std::string, TokenGroup> data;


        /**
         * @e attribute will be
         * - ( '{'/'}' ) when @e type is SUB_BRACKET
         * - ( '-'/'=' ) when @e type is FLAG
         * - ( a quote ) when @e type is STRING_QUOTE
         * ------------------------------------------
         *
         * When @e type is STRING_QUOTE, the char will contain the
         * type of the quote, which will either be ', ", or `
         */
        std::optional<char> attribute;


        /**
         * get the "syntax-highlighted" version of @e text
         * -----------------------------------------------
         *
         * the function will recursively run this function on every other Token
         * that the substitution contains when @e type is SUB_CONTENT
         */
        [[nodiscard]]
        auto get_highlighted() -> std::string;


        /**
         * a wrapper for std::get_if ran on @e data
         */
        template <typename Tp>
        [[nodiscard]]
        auto
        get_data() const -> Tp
        {
            return *std::get_if<Tp>(&this->data);
        }


        ~Token();
    };
}
