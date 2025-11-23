#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

#include "../error.hh"
#include "parser/types.hh"


namespace parser
{
    struct TokenGroup;


    namespace error
    {
        enum class Type : std::uint8_t
        {
            NONE,

            INVALID_COMMAND,
            UNCLOSED_QUOTE,
            UNCLOSED_BRACKET,

            INVALID_BRACKET,

            CORRUPTED_TOKEN,
            CORRUPTED_TOKEN_ATTRIBUTE,

            UNSUPPORTED_OPERATION,

            EMPTY_ARITHMETIC_EXPRESSION,
            EMPTY_SUBSTITUTION,
            EMPTY_STRING,
            EMPTY_PARAM,
        };


        constexpr auto
        type_to_string(Type t) -> std::string_view
        {
            /* clang-format off */
            switch (t)
            {
            case Type::NONE:
                return "parser::NO_ERROR";

            case Type::INVALID_COMMAND:
                return "parser::INVALID_COMMAND";

            case Type::UNCLOSED_QUOTE:
                return "parser::UNCLOSED_QUOTE";

            case Type::UNCLOSED_BRACKET:
                return "parser::UNCLOSED_BRACKET";

            case Type::INVALID_BRACKET:
                return "parser::INVALID_BRACKET";

            case Type::CORRUPTED_TOKEN:
                return "parser::CORRUPTED_TOKEN";

            case Type::CORRUPTED_TOKEN_ATTRIBUTE:
                return "parser::CORRUPTED_TOKEN_ATTRIBUTE";

            case Type::UNSUPPORTED_OPERATION:
                return "parser::UNSUPPORTED_OPERATION";

            case Type::EMPTY_ARITHMETIC_EXPRESSION:
                return "parser::EMPTY_ARITHMETIC_EXPRESSION";

            case Type::EMPTY_SUBSTITUTION:
                return "parser::EMPTY_SUBSTITUTION";

            case Type::EMPTY_STRING:
                return "parser::EMPTY_STRING";

            case Type::EMPTY_PARAM:
                return "parser::EMPTY_PARAM";
            }
            /* clang-format on */
        }


        [[nodiscard]]
        auto compute_real_index(const TokenGroup *group, const Token *tkn)
            -> std::size_t;


        template <Type T_ErrorType, typename... T_Args>
        [[nodiscard]]
        auto
        create(TokenGroup      &tokens,
               Token           &token,
               std::string_view fmt,
               T_Args &&...args) -> ::error::Info
        {
            ::error::Info err { std::string { type_to_string(T_ErrorType) },
                                fmt, std::forward<T_Args>(args)... };

            auto it { std::ranges::find(tokens.tokens, token) };
            if (it == tokens.tokens.end()) return err;

            const auto *toplevel { tokens.get_toplevel() };
            std::string text;
            try
            {
                if (const auto *str_ptr { token.get_data<std::string>() })
                    text = *str_ptr;
                else
                    text = "[NO DATA]";
            }
            catch (const std::bad_variant_access &)
            {
                text = "[NO DATA]";
            }

            err.set_error_context(tokens.source, toplevel->raw,
                                  compute_real_index(&tokens, &token),
                                  text.length());
            return err;
        }
    }
}
