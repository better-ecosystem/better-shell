#include "error.hh"

using error::Info;


namespace
{
    [[nodiscard]]
    auto
    index_to_line_column(const std::string &text, size_t index)
        -> std::spair<size_t>
    {
        size_t line { 0 };
        size_t col { 0 };

        size_t i { 0 };
        while (i < index && i < text.length())
        {
            if (text[i] == '\n')
            {
                line++;
                col = 0;
            }
            else
                col++;
            i++;
        }

        return { line, col };
    }
}


void
Info::set_error_context(const std::string &input_source,
                        const std::string &text,
                        size_t             idx,
                        size_t             len)
{
    if (input_source.empty() || text.empty()) return;

    m_error_pos    = index_to_line_column(text, idx);
    m_error_lines  = utils::str::split(text, '\n');
    m_input_source = input_source;
    m_error_len    = len;
}


auto
Info::create_pretty_message(bool force) -> std::string
{
    if (!force && !m_pretty_msg.empty()) [[likely]]
        return m_pretty_msg;

    m_pretty_msg = std::format("{}error:{} {}\n\n", color::ERROR, color::RESET,
                               m_error_type);

    /* if theres no further context */
    if (m_error_lines.empty()) [[unlikely]]
    {
        m_pretty_msg += std::format("  {}message:{} {} (no further context)",
                                    color::MESSAGE, color::RESET, m_msg);
        return m_pretty_msg;
    }

    m_pretty_msg
        += std::format("  ╭─[{}{}{}: {}:{}]\n", color::MESSAGE, m_input_source,
                       color::RESET, m_error_pos.first, m_error_pos.second);

    for (size_t i { 0 }; i <= m_error_pos.first && i < m_error_lines.size();
         i++)
    {
        /* get the length of right-padding */
        size_t padding_len { m_error_pos.second + m_msg.length()
                             - m_error_lines[i].length() };

        auto background { i % 2 == 0 ? color::LINE_BG : color::LINE_BG_ALT };
        auto line { std::format("{}{}{}{}{} │ {}{}{}\n", background,
                                color::LINE_NUM, i + 1, color::RESET,
                                background, m_error_lines[i],
                                std::string(padding_len, ' '), color::RESET) };

        if (i == m_error_pos.first)
        {
            std::string underline(m_error_pos.second, ' ');
            underline += color::ERROR;
            underline.append(m_error_len, '^');
            underline += color::RESET;

            line += std::format("  · {}{}{}\n"
                                "  · {}{}\n",
                                color::ERROR, underline, color::RESET,
                                std::string(m_error_pos.second, ' '), m_msg);
        }

        m_pretty_msg += line;
    }

    m_pretty_msg += "  ╰─";
    for (auto _ : utils::range<size_t>(0, m_error_pos.second))
        m_pretty_msg.append("─");

    RUN_FUNC_N(m_msg.length(), m_pretty_msg.append, _ % 2 ? "·" : " ");
    m_pretty_msg += "\n";

    return m_pretty_msg;
}
