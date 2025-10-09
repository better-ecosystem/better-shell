#pragma once
#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>

#include "parser/types.hh"
#include "print.hh"
#include "utils.hh"


namespace parser
{
    struct TokenGroup;


    enum class ErrorType : uint8_t
    {
        PARSER_ERROR_NONE,

        PARSER_FIRST_TOKEN_IS_NOT_COMMAND,
        PARSER_INVALID_COMMAND,
        PARSER_UNCLOSED_QUOTE,
        PARSER_UNCLOSED_BRACKET,

        PARSER_CORRUPTED_TOKEN,
        PARSER_CORRUPTED_TOKEN_ATTRIBUTE,

        PARSER_UNSUPPORTED_OPERATION,

        PARSER_EMPTY_SUBSTITUTION,
        PARSER_EMPTY_STRING,
        PARSER_EMPTY_PARAM,
    };


    constexpr auto
    ErrorType_to_string(ErrorType t) -> std::string_view
    {
        // clang-format off
        switch (t)
        {
        case ErrorType::PARSER_ERROR_NONE:
            return "parser::NO_ERROR";

        case ErrorType::PARSER_FIRST_TOKEN_IS_NOT_COMMAND:
            return "parser::INVALID_FIRST_TOKEN";

        case ErrorType::PARSER_INVALID_COMMAND:
            return "parser::INVALID_COMMAND";

        case ErrorType::PARSER_UNCLOSED_QUOTE:
            return "parser::UNCLOSED_QUOTE";

        case ErrorType::PARSER_UNCLOSED_BRACKET:
            return "parser::UNCLOSED_BRACKET";

        case ErrorType::PARSER_CORRUPTED_TOKEN:
            return "parser::CORRUPTED_TOKEN";

        case ErrorType::PARSER_CORRUPTED_TOKEN_ATTRIBUTE:
            return "parser::CORRUPTED_TOKEN_ATTRIBUTE";

        case ErrorType::PARSER_UNSUPPORTED_OPERATION:
            return "parser::UNSUPPORTED_OPERATION";

        case ErrorType::PARSER_EMPTY_SUBSTITUTION:
            return "parser::EMPTY_SUBSTITUTION";

        case ErrorType::PARSER_EMPTY_STRING:
            return "parser::EMPTY_STRING";

        case ErrorType::PARSER_EMPTY_PARAM:
            return "parser::EMPTY_PARAM";
        }
        // clang-format on
    }


    class Error
    {
    public:
        template <typename... T_Args>
        Error(TokenGroup      &tokens,
              ErrorType        type,
              std::string_view fmt,
              size_t           token_idx,
              T_Args &&...args)
            : m_type(type),
              m_message(std::vformat(
                  fmt, std::make_format_args(std::forward<T_Args>(args)...))),
              m_token_idx(token_idx)
        {
            if (m_token_idx >= tokens.tokens.size())
            {
                m_pretty = std::format(
                    "{}error:{} {} (no further context available)", m_red,
                    COLOR_RESET, m_message);
                return;
            }

            const Token &TOKEN { tokens.tokens.at(m_token_idx) };
            const size_t INDEX { compute_real_index(tokens, TOKEN.index) };
            const std::string &RAW { tokens.get_toplevel()->raw };

            const std::string *error_token_text { extract_error_token_string(
                TOKEN, m_token_idx) };
            if (error_token_text == nullptr) return;

            format_pretty_message(*error_token_text, RAW,
                                  line_column_from_offset(RAW, INDEX));
        }


        [[nodiscard]]
        auto get_message() -> std::string &;


        template <char T_Default, char... T_Options, typename... T_Args>
        static auto
        ask(std::string_view fmt, T_Args &&...args) -> char
        {
            std::string msg { std::vformat(fmt,
                                           std::make_format_args(args...)) };

            io::print("{}ask:{} {} [", m_blue, COLOR_RESET, msg);
            ((io::print("{}", (T_Options == T_Default
                                   ? static_cast<char>(std::toupper(T_Options))
                                   : T_Options))),
             ...);
            io::print("] ");

            constexpr std::array<char, sizeof...(T_Options)> VALID_OPTIONS {
                T_Options...
            };

            int ch { std::getchar() };
            std::cerr.put(ch);
            std::cerr.put('\n');

            if (ch == EOF || ch == 4) return T_Default;
            if (ch == '\n' || ch == '\r') return T_Default;

            for (char opt : VALID_OPTIONS)
                if (std::tolower(ch) == std::tolower(opt)) return opt;

            io::println(
                "{}error:{} invalid input, please enter the given options",
                m_red, COLOR_RESET);
            return ask<T_Default, T_Options...>(msg);
        }

    private:
        ErrorType   m_type;
        std::string m_message;
        size_t      m_token_idx;

        std::string m_pretty;

        /* subject to change after config implementation */
        static constexpr std::string_view m_red { ANSI_RGB_FG(253, 106, 106) };
        static constexpr std::string_view m_blue { ANSI_RGB_FG(70, 172, 173) };
        static constexpr std::string_view m_text_color { ANSI_RGB_FG(
            150, 150, 150) };

        static constexpr std::string_view m_bg { ANSI_RGB_BG(48, 48, 48) };
        static constexpr std::string_view m_bg_alt { ANSI_RGB_BG(75, 75, 75) };


        void format_pretty_message(const std::string &error_token_text,
                                   const std::string &top_level_raw_string,
                                   std::pair<size_t, size_t> position);


        [[nodiscard]]
        auto extract_error_token_string(const Token &token, size_t token_idx)
            -> const std::string *;


        [[nodiscard]]
        static auto compute_real_index(const TokenGroup &tokens,
                                       size_t            base_index) -> size_t;


        [[nodiscard]]
        static auto line_column_from_offset(const std::string &text,
                                            size_t             offset)
            -> std::pair<size_t, size_t>;
    };
}
