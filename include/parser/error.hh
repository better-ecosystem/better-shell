#pragma once
#include <cstdint>
#include <format>
#include <string>

#include "parser/types.hh"
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
            using namespace std::literals;

            if (m_token_idx >= tokens.tokens.size())
            {
                m_pretty = std::format(
                    "error: {} (no further context available)", m_message);
                return;
            }

            const Token &TOKEN { tokens.tokens.at(m_token_idx) };
            const size_t INDEX { compute_real_index(tokens, TOKEN.index) };
            const std::string &RAW { tokens.get_toplevel()->raw };

            const std::string *error_token_text { extract_error_token_string(
                TOKEN, m_token_idx) };

            format_pretty_message(*error_token_text, RAW,
                                  line_column_from_offset(RAW, INDEX));
        }


        [[nodiscard]]
        auto get_message() const -> std::string;

    private:
        ErrorType   m_type;
        std::string m_message;
        size_t      m_token_idx;

        std::string m_pretty;

        /* subject to change after config implementation */
        std::string m_red { ANSI_RGB_FG(253, 106, 106) };
        std::string m_blue { ANSI_RGB_FG(70, 172, 173) };


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
