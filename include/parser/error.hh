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

        PARSER_UNSUPPORTED_OPERATION
    };


    class Error
    {
    public:
        template <typename... T_Args>
        Error(const TokenGroup &tokens,
              ErrorType         type,
              std::string_view  fmt,
              size_t            token_idx,
              T_Args &&...args)
            : m_type(type),
              m_message(std::vformat(
                  fmt, std::make_format_args(std::forward<T_Args>(args)...))),
              m_token_idx(token_idx)
        {
            if (m_token_idx >= tokens.tokens.size())
            {
                m_pretty = std::format(
                    "error: {} (no further context available)", m_message);
                return;
            }

            const std::string &INPUT { tokens.raw };
            const Token       &TOKEN { tokens.tokens[m_token_idx] };

            std::string text;
            try
            {
                text = TOKEN.get_data<std::string>();
            }
            catch (const std::bad_variant_access &)
            {
                m_pretty = std::format(
                    "error: {} (token {} has no associated string data)",
                    m_message, m_token_idx);
                return;
            }

            const auto [LINE_NUM, COL_NUM] { line_column_from_offset(
                INPUT, TOKEN.index) };

            const auto LINES { utils::str::split_lines(INPUT) };

            const size_t UNDERLINE_LENGTH { std::max<size_t>(1,
                                                             text.length()) };

            const std::string UNDERLINE_MARKER {
                std::string(COL_NUM, ' ') + std::string(UNDERLINE_LENGTH, '^')
            };

            std::ostringstream oss;

            /* error N: <message> */
            oss << ANSI_RGB_FG(253, 106, 106) "error "
                << static_cast<int>(m_type) << COLOR_RESET ": " << m_message
                << "\n\n";

            oss << "  ╭─[" ANSI_RGB_FG(70, 172, 173) "shell input: " COLOR_RESET
                << std::format("{}:{}]\n", LINE_NUM + 1, COL_NUM + 1);
            oss << "  │" << "\n";

            size_t i { LINE_NUM > 0 ? LINE_NUM - 1 : 0 };

            for (; i <= std::min(LINE_NUM + 1, LINES.size() - 1); ++i)
            {
                oss << std::format("{} │ {}\n", i + 1, LINES[i]);

                if (i == LINE_NUM)
                {
                    oss << "  · " ANSI_RGB_FG(211, 79, 109) << UNDERLINE_MARKER
                        << '\n';

                    oss << COLOR_RESET "  · " << std::string(COL_NUM, ' ')
                        << m_message << "\n";
                }
            }

            std::string end_line { "  ╰─" };
            RUN_FUNC_N(UNDERLINE_MARKER.length() - UNDERLINE_LENGTH,
                       end_line.append, "─");
            RUN_FUNC_N(m_message.length(), end_line.append, _ % 2 ? "·" : " ");
            oss << end_line << '\n';

            m_pretty = oss.str();
        }


        [[nodiscard]]
        auto get_message() const -> std::string;

    private:
        ErrorType   m_type;
        std::string m_message;
        size_t      m_token_idx;

        std::string m_pretty;


        [[nodiscard]]
        static auto line_column_from_offset(const std::string &text,
                                            size_t             offset)
            -> std::pair<size_t, size_t>;
    };
}
