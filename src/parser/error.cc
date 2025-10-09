#include "parser/error.hh"

using parser::Error;


auto
Error::get_message() const -> std::string
{
    return m_pretty;
}


void
Error::format_pretty_message(const std::string        &error_token_text,
                             const std::string        &top_level_raw_string,
                             std::pair<size_t, size_t> position)
{
    const auto   LINES { utils::str::split(top_level_raw_string, '\n') };
    const size_t UNDERLINE_LENGTH { MAXB(1, error_token_text.length()) };
    const auto   UNDERLINE_MARKER {
        std::string(position.second, ' ') + std::string(UNDERLINE_LENGTH, '^'),
    };

    m_pretty = std::format("{}error{}: {}\n"
                           "\n"
                           "  ╭─[{}{}{}: {}:{}]\n",
                           m_red, COLOR_RESET, ErrorType_to_string(m_type),
                           m_blue, "shell input", COLOR_RESET, position.first,
                           position.second);


    size_t i { position.first > 0 ? position.first - 1 : 0 };
    for (; i <= std::min(position.first + 1, LINES.size() - 1); i++)
    {
        std::string bg { i % 2 == 0 ? m_bg : m_bg_alt };
        std::string msg { std::format("{}{}{}{}{} │ {}", bg, m_text_color,
                                      i + 1, COLOR_RESET, bg, LINES[i]) };

        size_t space_amount { position.second + m_message.length()
                              - LINES[i].length() };
        msg += std::string(space_amount, ' ') + COLOR_RESET + '\n';

        if (i == position.first)
        {
            msg += std::format("  · {}{}{}\n"
                               "  · {}{}\n",
                               m_red, UNDERLINE_MARKER, COLOR_RESET,
                               std::string(position.second, ' '), m_message);
        }

        m_pretty += msg;
    }

    m_pretty += "  ╰─";
    for (auto _ : utils::range<size_t>(0, position.second))
        m_pretty.append("─");

    RUN_FUNC_N(m_message.length(), m_pretty.append, _ % 2 ? "·" : " ");
    m_pretty += "\n";
}


auto
Error::extract_error_token_string(const Token &token, size_t token_idx)
    -> const std::string *
{
    try
    {
        if (const auto *str_ptr { token.get_data<std::string>() })
            return str_ptr;

        m_pretty
            = std::format("error: {} (token {} has no associated string data)",
                          m_message, token_idx);
        return nullptr;
    }
    catch (const std::bad_variant_access &)
    {
        m_pretty
            = std::format("error: {} (token {} has no associated string data)",
                          m_message, token_idx);
        return nullptr;
    }
}


auto
Error::compute_real_index(const TokenGroup &tokens, size_t base_index) -> size_t
{
    size_t            real_index { base_index };
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
                real_index += t.get_data<shared_tokens>()->get()->raw.length();
            else
                real_index += t.get_data<std::string>()->length();
        }

        current = parent_ptr.get();
        real_index++;
    }

    return real_index;
}


auto
Error::line_column_from_offset(const std::string &text, size_t offset)
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
