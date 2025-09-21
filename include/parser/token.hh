#pragma once
#ifndef __BetterShell__parser_parser_hh
#define __BetterShell__parser_parser_hh
#include <cstdint>
#include <string>
#include <vector>


namespace parser
{
    enum class TokenType : uint8_t
    {
        COMMAND,    /* < Command to run */
        ARGUMENT,   /* < Command argument (cmd arg --flag) */
        FLAG,
        SUBSTITUTE, /* < ${cmd, ...} */
        OPERATOR,   /* < +, -, ||, ... */
        STRING,     /* < Pure string */
        END,        /* < EOF */
    };


    struct Token
    {
        TokenType type;
        std::string data;


        [[nodiscard]]
        static auto tokenize( std::string &p_str ) -> std::vector<Token>;
    };
}

#endif /* __BetterShell__parser_parser_hh */