#include <csignal>
#include <cwchar>
#include <iostream>

#include <unistd.h>
#include <utf8.h>

#include "input/terminal.hh"
#include "print.hh"
#include "utils.hh"

using input::term::Handler;


namespace
{
    /**
     * @return `std::nullopt` on EOT, or a valid `std::string`
     *          containing the ANSI sequence.
     */
    [[nodiscard]]
    auto
    get_ansi_sequence(std::streambuf *sbuf) -> std::optional<std::string>
    {
        std::array<char, 8> buff;
        for (size_t i { 0 }; i < buff.size(); i++)
        {
            if (sbuf->sgetc() == EOT) return std::nullopt;

            buff[i] = static_cast<char>(sbuf->sbumpc());
            if ((buff[i] >= 'A' && buff[i] <= 'Z') || buff[i] == '~')
            {
                buff[++i] = '\0';
                break;
            }
        }

        return buff.data();
    }
}


Handler *Handler::m_handler_instance { nullptr };
Handler::Handler(std::istream *stream) : m_stream(stream), m_is_term(false)
{
    if (stream->rdbuf() == std::cin.rdbuf() && isatty(STDIN_FILENO) != 0)
    {
        tcgetattr(STDIN_FILENO, &m_old_term);

        termios newt { m_old_term };

        newt.c_lflag    &= ~(ICANON | ECHO);
        newt.c_cc[VMIN]  = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        m_is_term = true;

        m_handler_instance = this;
        std::signal(SIGINT, Handler::sigint_handler);
    }
}


Handler::~Handler()
{
    if (m_is_term) tcsetattr(STDIN_FILENO, TCSANOW, &m_old_term);
}


auto
Handler::is_active() const -> bool
{
    return m_is_term;
}


void
Handler::reset()
{
    m_pos.x      = 0;
    m_pos.y      = 0;
    m_pos.last_x = 0;
}


void
Handler::show_prompt()
{
    io::print("$ ");
}


auto
Handler::handle(const unsigned char &current,
                std::string         &str,
                std::streambuf      *sbuf) -> ReturnType
{
    if (!m_is_term) return RETURN_NONE;

    if (current == '\\')
    {
        m_escaped = true;
        io::print("{}", current);
        insert_char_to_cursor(str, current);
        return RETURN_CONTINUE;
    }

    if (current == '\n')
    {
        io::println("");

        if (m_escaped)
        {
            insert_char_to_cursor(str, current);
            return RETURN_CONTINUE;
        }
        return RETURN_DONE;
    }

    /* ANSI escape code */
    if (current == '\033')
    {
        if (!handle_arrow(str, sbuf)) return RETURN_EXIT;
        return RETURN_CONTINUE;
    }

    /* Backspace */
    if (current == 0x7F || current == 0x08)
    {
        handle_backspace(str, current == 0x08);
        return RETURN_CONTINUE;
    }

    if (utils::is_leading_byte(current))
    {
        m_u8_buffer      += current;
        m_u8_expected_len = utils::utf8_get_expected_length(current);

        return RETURN_CONTINUE;
    }

    if (utils::is_continuation_byte(current))
    {
        m_u8_buffer += current;

        if (m_u8_buffer.size() == m_u8_expected_len)
        {
            io::print("\033[s{}{}\033[u\033[C", m_u8_buffer,
                      str.substr(m_pos.x));
            insert_u8_to_cursor(str);
            m_u8_buffer.clear();
            m_u8_expected_len = 0;
        }

        return RETURN_CONTINUE;
    }

    if (utils::is_ascii_byte(current))
    {
        io::print("\033[s{}{}\033[u\033[C", static_cast<char>(current),
                  str.substr(m_pos.x));
        insert_char_to_cursor(str, current);
    }


    m_escaped = false;

    return RETURN_NONE;
}


void
Handler::insert_char_to_cursor(std::string &str, unsigned char c)
{
    size_t idx { m_pos.get_string_idx(str) };
    str.insert(idx, 1, c);

    if (c == '\n')
    {
        m_pos.x = 0;
        m_pos.y++;
    }
    else
        m_pos.x++;
}


void
Handler::insert_u8_to_cursor(std::string &str)
{
    size_t idx { m_pos.get_string_idx(str) };
    str.insert(idx, m_u8_buffer);

    try
    {
        auto       it { m_u8_buffer.begin() };
        const auto end { m_u8_buffer.end() };

        char32_t codepoint = utf8::next(it, end);

        int width { wcwidth(static_cast<wchar_t>(codepoint)) };
        if (width < 0) width = 1;

        m_pos.x += width;
    }
    catch (const utf8::invalid_utf8 &e)
    {
        m_pos.x += 1;
    }
    catch (...)
    {
        m_pos.x += 1;
    }
}


void
Handler::handle_backspace(std::string &str, bool ctrl)
{
    if (m_pos.is_zero()) return;

    if (m_pos.x > 0)
    {
        size_t idx { m_pos.get_string_idx(str) };
        str.erase(idx - 1, 1);
        m_pos.x--;

        io::print("\033[D");
        io::print("\033[s");
        io::print("{} ", str.substr(idx - 1));
        io::print("\033[u");
    }
}


auto
Handler::handle_arrow(const std::string &str, std::streambuf *sbuf) -> bool
{
    auto seq_buff { get_ansi_sequence(sbuf) };
    if (!seq_buff) return false;

    const std::string &seq { *seq_buff };
    const bool         ctrl { seq.starts_with("[1;5") };
    const auto         dir { Cursor::Direction(seq.back()) };

    /* dir is either C/D */
    if (dir >= 'C') return m_pos.handle_arrows(dir, str, ctrl);
    return m_pos.handle_arrows(dir, str, ctrl) || handle_history(str);
}


auto
Handler::handle_history(const std::string &current) -> bool
{
    return true;
}


void
Handler::sigint_handler(int sig)
{
    if (sig == SIGINT && m_handler_instance != nullptr)
    {
        if (!m_handler_instance->is_active()) return;

        io::println("^C");
        std::signal(SIGINT, Handler::sigint_handler);
    }
}
