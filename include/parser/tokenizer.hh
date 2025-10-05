#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace parser
{
    enum class TokenType : uint8_t
    {
        COMMAND,  /* < Command to run */
        ARGUMENT, /* < Command argument (cmd arg --flag) */
        FLAG,
        SUBSTITUTE, /* < ${cmd, ...} */
        OPERATOR,   /* < +, -, ||, ... */
        STRING,     /* < Pure string */
        FAIL,       /* < Bad strings / can't decide */
        END,        /* < EOF */
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


    struct Token
    {
        TokenType                                     type;
        std::variant<std::string, std::vector<Token>> data;

        [[nodiscard]]
        static auto tokenize(std::string &str) -> std::vector<Token>;
    };


    inline std::unordered_map<std::string, std::filesystem::path> BINARIES;

    void get_executable();
}
