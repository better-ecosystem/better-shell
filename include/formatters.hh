#pragma once
#include <format>
#include <vector>

#include "parser/tokenizer.hh"


/* std::vector formatter */

template <typename Tp>
    requires requires(const Tp &val, std::format_context &ctx) {
        std::formatter<Tp> {}.format(val, ctx);
    }

struct std::formatter<std::vector<Tp>> : std::formatter<std::string>
{
    auto
    format(const std::vector<Tp> &items, format_context &ctx) const
    {
        std::string out { "[ " };
        bool        first { true };

        for (const auto &item : items)
        {
            if (!first) out += ", ";
            first = false;
            out  += std::format("{}", item);
        }

        out += " ]";
        return std::formatter<std::string>::format(out, ctx);
    }
};

/* Token formatter for debugging purposes */

template <>
struct std::formatter<parser::TokenType> : std::formatter<std::string_view>
{
    auto
    format(parser::TokenType type, format_context &ctx) const
    {
        std::string_view name { parser::token_type_to_string(type) };
        return std::formatter<std::string_view>::format(name, ctx);
    }
};


template <> struct std::formatter<parser::Token> : std::formatter<std::string>
{
    auto
    format(const parser::Token &tok, format_context &ctx) const
    {
        std::string out { std::format("{}(", tok.type) };

        std::visit(
            [&](const auto &val) -> auto
            {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, std::string>)
                {
                    out += std::format("\"{}\"", val);
                    return;
                }

                if constexpr (std::is_same_v<T, std::vector<parser::Token>>)
                {
                    std::vector<parser::Token> tokens { val };

                    std::string str { " " };
                    bool        first { true };

                    for (const auto &token : tokens)
                    {
                        if (!first) str += ", ";
                        first = false;
                        str  += std::format("{}", token);
                    }

                    str += " ";
                    out += str;
                }
            },
            tok.data);

        out += ")";
        return std::formatter<std::string>::format(out, ctx);
    }
};
