#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <json/value.h>


namespace parser
{
    enum class TokenType : uint8_t
    {
        COMMAND,    /* < Command to run */
        ARGUMENT,   /* < Command argument (cmd arg) */
        FLAG,       /* < Command flag (cmd --flag) */
        SUBSTITUTE, /* < ${cmd, ...} */
        OPERATOR,   /* < +, -, ||, ... */
        STRING,     /* < Pure string */
        FAIL,       /* < Bad strings / can't decide */
        END,        /* < EOF */
        UNKNOWN,
    };


    [[nodiscard]]
    constexpr auto
    token_type_to_string(TokenType type) noexcept -> std::string_view
    {
        switch (type)
        {
        case parser::TokenType::COMMAND:    return "COMMAND";
        case parser::TokenType::ARGUMENT:   return "ARGUMENT";
        case parser::TokenType::FLAG:       return "FLAG";
        case parser::TokenType::SUBSTITUTE: return "SUBSTITUTE";
        case parser::TokenType::OPERATOR:   return "OPERATOR";
        case parser::TokenType::STRING:     return "STRING";
        case parser::TokenType::FAIL:       return "FAIL";
        case parser::TokenType::END:        return "END";
        default:                            return "UNKNOWN";
        }
    }


    [[nodiscard]]
    constexpr auto
    token_type_from_string(std::string_view str) noexcept -> TokenType
    {
        if (str == "COMMAND") return parser::TokenType::COMMAND;
        if (str == "ARGUMENT") return parser::TokenType::ARGUMENT;
        if (str == "FLAG") return parser::TokenType::FLAG;
        if (str == "SUBSTITUTE") return parser::TokenType::SUBSTITUTE;
        if (str == "OPERATOR") return parser::TokenType::OPERATOR;
        if (str == "STRING") return parser::TokenType::STRING;
        if (str == "FAIL") return parser::TokenType::FAIL;
        if (str == "END") return parser::TokenType::END;

        return parser::TokenType::UNKNOWN;
    }


    struct Token;
    struct Tokens
    {
        std::vector<Token> tokens;
        std::string        raw;

        [[nodiscard]]
        static auto tokenize(std::string &str) -> Tokens;


        [[nodiscard]]
        static auto from_json(const Json::Value &json) -> Tokens;


        [[nodiscard]]
        static auto to_json(const Tokens &tokens) -> Json::Value;


        [[nodiscard]]
        auto operator->() -> std::vector<Token> *;
    };


    struct Token
    {
        TokenType                         type;
        std::variant<std::string, Tokens> data;
    };


    inline std::unordered_map<std::string, std::filesystem::path> BINARIES;

    void get_executable();
}
