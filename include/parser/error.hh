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
        enum class Type : uint8_t
        {
            NONE,

            INVALID_COMMAND,
            UNCLOSED_QUOTE,
            UNCLOSED_BRACKET,

            CORRUPTED_TOKEN,
            CORRUPTED_TOKEN_ATTRIBUTE,

            UNSUPPORTED_OPERATION,

            EMPTY_SUBSTITUTION,
            EMPTY_STRING,
            EMPTY_PARAM,
        };


        constexpr auto
        type_to_string(Type t) -> std::string_view
        {
            // clang-format off
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

            case Type::CORRUPTED_TOKEN:
                return "parser::CORRUPTED_TOKEN";

            case Type::CORRUPTED_TOKEN_ATTRIBUTE:
                return "parser::CORRUPTED_TOKEN_ATTRIBUTE";

            case Type::UNSUPPORTED_OPERATION:
                return "parser::UNSUPPORTED_OPERATION";

            case Type::EMPTY_SUBSTITUTION:
                return "parser::EMPTY_SUBSTITUTION";

            case Type::EMPTY_STRING:
                return "parser::EMPTY_STRING";

            case Type::EMPTY_PARAM:
                return "parser::EMPTY_PARAM";
            }
            // clang-format on
        }


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

            auto real_idx { static_cast<size_t>(it - tokens.tokens.begin()) };

            /* get the real index */
            const TokenGroup *current { &tokens };
            while (auto parent_ptr { current->parent.lock() })
            {
                /* in which index of tokens is the child token placed
                   in the parent tokens list
                */
                size_t child_idx { 0 };
                for (; child_idx < parent_ptr->tokens.size(); child_idx++)
                {
                    auto t { parent_ptr->tokens[child_idx] };
                    if (t.type == TokenType::SUB_CONTENT)
                    {
                        const auto *sub { t.get_data<shared_tokens>() };
                        if (sub != nullptr && sub->get() == current) break;
                    }
                }

                /* now we need to add all of the string data's lengths until
                   the child index
                */
                for (size_t i { 0 }; i < child_idx; ++i)
                {
                    Token &t { parent_ptr->tokens[i] };
                    if (t.type == TokenType::SUB_CONTENT)
                        real_idx
                            += t.get_data<shared_tokens>()->get()->raw.length();
                    else
                        real_idx += t.get_data<std::string>()->length();
                }

                current = parent_ptr.get();
                real_idx++;
            }

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

            err.set_error_context(toplevel->source, toplevel->raw, real_idx,
                                  text.length());
            return err;
        }
    }
}
