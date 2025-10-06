#pragma once
#include <cstdint>
#include <format>
#include <string>

#include "parser/types.hh"
#include "utils.hh"


namespace
{
    [[nodiscard]]
    auto
    line_column_from_offset(const std::string &text, size_t offset)
        -> std::pair<size_t, size_t>
    {
        size_t line { 0 };
        size_t col { 0 };

        for (size_t i { 0 }; i < offset && i < text.size(); i++)
        {
            if (text[i] == '\n')
            {
                line++;
                col = 0;
            }
            else
                col++;
        }

        return { line, col };
    }
}


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
            const std::string &TEXT  { TOKEN.get_data<std::string>()};

            size_t search_start { 0 };
            for (size_t i { 0 }; i < m_token_idx; ++i)
            {
                const std::string TEXT {
                    tokens.tokens[i].get_data<std::string>()
                };

                auto found { INPUT.find(TEXT, search_start) };
                if (found != std::string::npos)
                    search_start = found + TEXT.length();
            }

            size_t token_offset { INPUT.find(TEXT, search_start) };
            if (token_offset == std::string::npos) token_offset = 0;

            auto [line_num,
                  col_num] { line_column_from_offset(INPUT, token_offset) };
            auto        lines { utils::str::split_lines(INPUT) };
            std::string underline_marker {
                std::string(col_num, ' ')
                + std::string(std::max<size_t>(1, TEXT.size()), '^')
            };

            std::ostringstream oss;

            oss << ANSI_RGB_FG(211, 79, 109) "error " << static_cast<int>(type)
                << COLOR_RESET ": " << m_message << "\n\n";
            oss << "  ╭─ " ANSI_RGB_FG(70, 172, 173) "shell input: " COLOR_RESET
                << std::format("{}:{}\n", line_num + 1, col_num + 1);
            oss << "  │" << "\n";

            for (size_t i { std::max<size_t>(0, line_num - 1) };
                 i <= std::min(line_num + 1, lines.size() - 1); i++)
            {
                oss << std::format("{} │ {}\n", i + 1, lines[i]);
                if (i == line_num)
                {
                    oss << "  │   " ANSI_RGB_FG(211, 79, 109)
                        << underline_marker << " " COLOR_RESET << m_message
                        << "\n";
                }
            }

            m_pretty = oss.str();
        }


        [[nodiscard]]
        auto get_message() const -> std::string;

    private:
        ErrorType   m_type;
        std::string m_message;
        size_t      m_token_idx;

        std::string m_pretty;
    };
}
