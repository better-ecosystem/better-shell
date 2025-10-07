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
         * the parameters given to a flag/argument
         */
        PARAMETER,

        /**
         * operators, this includes commas, stars and such
         */
        OPERATOR,

        UNKNOWN,

        /* SUBSTITUTION TOKEN TYPE */

        /**
         * the bracket of the substitution, can either be OPEN or CLOSE,
         * on OPEN the text shall contain {,
         * and on CLOSE it will only contain }
         */
        SUB_BRACKET,

        /**
         * the content of the substitution, which will contain another tokens
         */
        SUB_CONTENT,

        /* STRING TOKEN TYPE */

        /**
         * the quote of the string, should be "
         */
        STRING_QUOTE,

        /**
         * the content of the string, not including the quotation marks.
         */
        STRING_CONTENT,
    };


    enum class OperatorType : uint8_t
    {
        /** ,
         * works like how zsh treats commas i.e., it only works
         * when used inside a substitute content
         */
        COMMA,


        /** *
         * used for globbing, i.e., works the same way like it does in other
         * shells, \* will make * become just a star, and if "calc"
         * is the command for the current TokenGroup will make it turn
         * into just a regular star
         */
        WILDCARD,


        /** proc | proc
         * redirects the stdout of a process on the left hand side
         * to the stdin of the process on the right hand side
         */
        PIPE,


        /** $name
         * substitute the space occupied by the string "$name" to be the
         * variable's value
         */
        SUBSTITUTE,


        /** bool && bool
         * returns true when left hand side and right hand side is true
         */
        LOGICAL_AND,


        /** bool || bool
         * returns true when either left hand side and right hand side is true
         */
        LOGICAL_OR,


        /** !bool
         * turns true into false, false into true
         */
        LOGICAL_NOT,


        /** ;
         * used to separate tokens, they're ran after one another
         */
        SEPARATOR_SEQUENCE,


        /** :
         * used to separate tokens, they're ran simultaneously on different
         * threads, output will be shown like
         *
         * proccess stderr/stdout [idx]: ...
         *
         * and for exit code, it will all be shown after every process
         * has finished, shown like
         *
         * proccess code [idx]: N
         * proccess code [idy]: N
         * proccess code [idz]: N
         */
        SEPARATOR_MULTI,


        NONE,
    };


    constexpr auto
    TokenType_to_string(TokenType t) noexcept -> std::string_view
    {
        switch (t)
        {
        case TokenType::COMMAND:        return "Type::COMMAND";
        case TokenType::ARGUMENT:       return "Type::ARGUMENT";
        case TokenType::FLAG:           return "Type::FLAG";
        case TokenType::PARAMETER:      return "Type::PARAMETER";
        case TokenType::OPERATOR:       return "Type::OPERATOR";
        case TokenType::UNKNOWN:        return "Type::UNKNOWN";
        case TokenType::SUB_BRACKET:    return "Type::SUB_BRACKET";
        case TokenType::SUB_CONTENT:    return "Type::SUB_CONTENT";
        case TokenType::STRING_QUOTE:   return "Type::STRING_QUOTE";
        case TokenType::STRING_CONTENT: return "Type::STRING_CONTENT";
        };
        return "Type::UNKNOWN";
    }


    constexpr auto
    OperatorType_to_string(OperatorType t) noexcept -> std::string_view
    {
        switch (t)
        {
        case OperatorType::COMMA:              return "Op::COMMA";
        case OperatorType::WILDCARD:           return "Op::WILDCARD";
        case OperatorType::PIPE:               return "Op::PIPE";
        case OperatorType::SUBSTITUTE:         return "Op::SUBSTITUTE";
        case OperatorType::LOGICAL_AND:        return "Op::LOGICAL_AND";
        case OperatorType::LOGICAL_OR:         return "Op::LOGICAL_OR";
        case OperatorType::LOGICAL_NOT:        return "Op::LOGICAL_NOT";
        case OperatorType::SEPARATOR_SEQUENCE: return "Op::SEPARATOR_SEQUENCE";
        case OperatorType::SEPARATOR_MULTI:    return "Op::SEPARATOR_MULTI";
        case OperatorType::NONE:               return "Op::NONE";
        }
        return "Op::UNKNOWN";
    }


    struct Token;
    class Error;


    /**
     * a container for @e Token
     */
    struct TokenGroup : std::enable_shared_from_this<TokenGroup>
    {
        std::vector<Token> tokens;
        std::string        raw;


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
        auto verify_syntax() -> std::optional<Error>;


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
        auto to_json() -> Json::Value;


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
        size_t index;


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
        OperatorType operator_type;


        Token() = default;
        Token(TokenType type, size_t idx, std::string data);
        Token(TokenType type, size_t idx, const shared_tokens &data);


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
