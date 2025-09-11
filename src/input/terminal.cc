#include <iostream>
#include <csignal>

#include <unistd.h>

#include "input/terminal.hh"
#include "print.hh"

using input::term::Handler;


namespace
{
    /**
     * @return `std::nullopt` on EOT, or a valid `std::string`
     *          containing the ANSI sequence.
     */
    [[nodiscard]]
    auto
    get_ansi_sequence( std::streambuf *p_sbuf ) -> std::optional<std::string>
    {
        std::array<char, 8> buff;
        for (size_t i { 0 }; i < buff.size(); i++) {
            if (p_sbuf->sgetc() == EOT) return std::nullopt;

            buff[i] = static_cast<char>(p_sbuf->sbumpc());
            if ((buff[i] >= 'A' && buff[i] <= 'Z') || buff[i] == '~') {
                buff[++i] = '\0';
                break;
            }
        }

        return buff.data();
    }
}


Handler *Handler::m_handler_instance { nullptr };
Handler::Handler( std::istream *p_stream ) :
    m_stream(p_stream),
    m_is_term(false)
{
    if (p_stream->rdbuf() == std::cin.rdbuf() && isatty(STDIN_FILENO) != 0) {
        tcgetattr(STDIN_FILENO, &m_old_term);

        termios newt { m_old_term };

        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        m_is_term = true;

        m_handler_instance = this;
        std::signal(SIGINT, Handler::sigint_handler);
    }
}


Handler::~Handler()
{ if (m_is_term) tcsetattr(STDIN_FILENO, TCSANOW, &m_old_term); }


auto
Handler::is_active() const -> bool
{ return m_is_term; }


void
Handler::reset()
{
    m_pos.x = 0;
    m_pos.y = 0;
    m_pos.last_x = 0;
}


void
Handler::show_prompt()
{
    io::print("$ ");
}


auto
Handler::handle( const char     &p_current,
                 std::string    &p_str,
                 std::streambuf *p_sbuf ) -> ReturnType
{
    if (!m_is_term) return RETURN_NONE;

    if (p_current == '\\') {
        m_escaped = true;
        io::print("{}", p_current);
        insert_char_to_cursor(p_str, p_current);
        return RETURN_CONTINUE;
    }

    if (p_current == '\n') {
        io::println("");

        if (m_escaped) {
            insert_char_to_cursor(p_str, p_current);
            return RETURN_CONTINUE;
        }
        return RETURN_DONE;
    }

    /* ANSI escape code */
    if (p_current == '\033') {
        if (!handle_arrow(p_str, p_sbuf)) return RETURN_EXIT;
        return RETURN_CONTINUE;
    }

    /* Backspace */
    if (p_current == 0x7F || p_current == 0x08) {
        handle_backspace(p_str, p_current == 0x08);
        return RETURN_CONTINUE;
    }

    io::print("\033[s{}{}\033[u\033[C", p_current, p_str.substr(m_pos.x));
    insert_char_to_cursor(p_str, p_current);

    m_escaped = false;

    return RETURN_NONE;
}


void
Handler::insert_char_to_cursor( std::string &p_str, char p_c )
{
    size_t idx { m_pos.get_string_idx(p_str) };
    p_str.insert(idx, 1, p_c);

    if (p_c == '\n') {
        m_pos.x = 0;
        m_pos.y++;
    } else m_pos.x++;
}


void
Handler::handle_backspace( std::string &p_str, bool p_ctrl )
{
    if (m_pos.is_zero()) return;

    if (m_pos.x > 0) {
        size_t idx { m_pos.get_string_idx(p_str) };
        p_str.erase(idx - 1, 1);
        m_pos.x--;

        io::print("\033[D");
        io::print("\033[s");
        io::print("{} ", p_str.substr(idx - 1));
        io::print("\033[u");
    }
}


auto
Handler::handle_arrow( const std::string &p_str,
                       std::streambuf    *p_sbuf ) -> bool
{
    auto seq_buff { get_ansi_sequence(p_sbuf) };
    if (!seq_buff) return false;

    const std::string &seq { *seq_buff };
    const bool ctrl { seq.starts_with("[1;5") };
    const auto dir  { CursorPosition::Direction(seq.back()) };

    /* dir is either C/D */
    if (dir >= 'C')
        return m_pos.handle_arrows(dir, p_str, ctrl);
    return m_pos.handle_arrows(dir, p_str, ctrl) || handle_history(p_str);
}


auto
Handler::handle_history( const std::string &p_current ) -> bool
{
    return true;
}


void
Handler::sigint_handler( int p_sig )
{
    if (p_sig == SIGINT && m_handler_instance != nullptr) {
        if (!m_handler_instance->is_active()) return;

        io::println("^C");
        std::signal(SIGINT, Handler::sigint_handler);
    }
}