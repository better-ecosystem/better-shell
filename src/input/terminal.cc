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
     * reads an ANSI sequence from a stream buffer @p sbuf
     * ---------------------------------------------------
     *
     * returns an ANSI sequence starting from '[',
     * or an empty string on EOT/EOF
     */
    [[nodiscard]]
    auto
    get_ansi_sequence(std::streambuf *sbuf) -> std::string
    {
        std::array<char, 8> buff;
        for (size_t i { 0 }; i < buff.size(); i++)
        {
            int c { sbuf->sgetc() };
            if (c == EOT || c == EOF) return "";

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

        m_history = std::make_unique<history::Handler>("");
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
    std::string HOME { utils::getenv("HOME") };
    std::string path { std::filesystem::current_path().string() };

    size_t pos { path.find(HOME) };
    path.replace(pos, HOME.length(), "~");

    io::print("[{}]$ ", path);
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
        io::print("{}", static_cast<char>(current));
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

        m_history->push_back(str);
        m_history->reset();
        return RETURN_DONE;
    }

    /* ANSI escape code */
    if (current == '\033')
    {
        if (!handle_ansi(str, sbuf)) return RETURN_EXIT;
        return RETURN_CONTINUE;
    }

    /* Backspace */
    if (current == 0x7F || current == 0x08)
    {
        handle_backspace(str, current == 0x08);
        return RETURN_CONTINUE;
    }

    if (utils::utf8::is_leading_byte(current))
    {
        m_u8_buffer      += current;
        m_u8_expected_len = utils::utf8::get_expected_length(current);

        return RETURN_CONTINUE;
    }

    if (utils::utf8::is_continuation_byte(current))
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

    if (utils::utf8::is_ascii_byte(current))
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

        if (!ctrl)
        {
            str.erase(idx - 1, 1);
            m_pos.x--;

            io::print("\033[D");
            io::print("\033[s");
            io::print("{} ", str.substr(idx - 1));
            io::print("\033[u");
            return;
        }

        size_t first { utils::str::move_idx_to_direction(str, idx, -1) };

        str.erase(first, idx - first);
        m_pos.x -= (idx - first);

        RUN_FUNC_N(idx - first, io::print, "\033[D")
        io::print("\033[s");
        io::print("{}{}", str.substr(first), std::string(idx - first, ' '));
        io::print("\033[u");
    }
}


auto
Handler::handle_ansi(std::string &str, std::streambuf *sbuf) -> bool
{
    std::string seq { get_ansi_sequence(sbuf) };
    if (seq.empty()) return false;

    const bool CTRL { utils::ansi::is_ctrl_pressed(seq) };

    if (utils::ansi::is_arrow(seq))
    {
        const auto DIRECTION { Cursor::Direction(seq.back()) };
        return m_pos.handle_arrows(DIRECTION, str, CTRL)
            || handle_history(DIRECTION, str);
    }

    if (int home_end { utils::ansi::is_home_end(seq) }; home_end != 0)
    {
        return m_pos.handle_home_end(home_end, str, CTRL);
    }
    return true;
}


auto
Handler::handle_history(Cursor::Direction direction, std::string &current_text)
    -> bool
{
    if (direction != Cursor::DIR_DOWN && direction != Cursor::DIR_UP)
        return false;
    if (m_current_text.empty()) m_current_text = current_text;

    std::string before { current_text };
    if (direction == Cursor::DIR_DOWN)
    {
        auto text { m_history->get_next() };
        if (!text)
        {
            current_text = m_current_text;
            m_current_text.clear();
            return true;
        }
        current_text = *text;
    }

    if (direction == Cursor::DIR_UP)
    {
        auto text { m_history->get_prev() };
        if (!text) return true;
        current_text = *text;
    }

    io::print("\r\033[K");

    show_prompt();
    io::print("{}", current_text);

    int64_t len { static_cast<int64_t>(before.length())
                  - static_cast<int64_t>(current_text.length()) };
    if (len > 0) io::print("{}", std::string(len, ' '));

    m_pos.x = utf8::distance(current_text.begin(), current_text.end());
    m_pos.y = 0;

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
