#include <algorithm>
#include <iostream>
#include <csignal>

#include <unistd.h>

#include "input/terminal.hh"
#include "print.hh"
#include "utils.hh"

using input::term::Handler;


namespace
{
    [[nodiscard]]
    auto
    xy_to_character_index_u8( const std::string &p_text,
                              size_t             p_x,
                              size_t             p_y ) -> size_t
    {
        size_t line { 0 };
        size_t idx  { 0 };

        for (size_t i { 0 }; i < p_text.size();) {
            if (line == p_y) {
                size_t char_x { 0 };

                while (i < p_text.size() && char_x < p_x && p_text[i] != '\n') {
                    unsigned char c { static_cast<unsigned char>(p_text[i]) };

                    int advance {
                        (c < 0x80) ? 1 :
                        (c < 0xE0) ? 2 :
                        (c < 0xF0) ? 3 : 4 };

                    i += advance;
                    char_x++; idx++;
                }

                if (char_x != p_x)
                    return 0;
                return idx;
            }

            while (i < p_text.size() && p_text[i] != '\n') {
                unsigned char c { static_cast<unsigned char>(p_text[i]) };
                int advance {
                    (c < 0x80) ? 1 :
                    (c < 0xE0) ? 2 :
                    (c < 0xF0) ? 3 : 4 };
                i += advance;
                idx++;
            }

            if (i < p_text.size() && p_text[i] == '\n') {
                i++; idx++; line++;
            }
        }

        return 0;
    }


    /**
     * @return `std::nullopt` on EOT, or a valid `std::string`
     *          containing the ANSI sequence.
     */
    [[nodiscard]]
    auto
    get_ansi_sequence( std::streambuf *p_sbuf ) -> std::optional<std::string>
    {
        std::array<char, 8> buff;
        for (uint16_t i { 0 }; i < buff.size(); i++) {
            if (p_sbuf->sgetc() == EOT) return std::nullopt;

            buff[i] = p_sbuf->sbumpc();
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
    if (p_stream->rdbuf() == std::cin.rdbuf() && isatty(STDIN_FILENO)) {
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
Handler::is_active() -> bool
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

    io::print("{}", p_current);
    insert_char_to_cursor(p_str, p_current);

    return RETURN_NONE;
}


void
Handler::insert_char_to_cursor( std::string &p_str, char p_c )
{
    size_t idx { xy_to_character_index_u8(p_str, m_pos.x, m_pos.y) };
    p_str.insert(idx, 1, p_c);

    m_pos.x++;
}


void
Handler::handle_backspace( std::string &p_str, bool p_ctrl )
{
    if (m_pos.is_zero()) return;

    if (m_pos.x > 0) {
        size_t idx { xy_to_character_index_u8(p_str, m_pos.x, m_pos.y) };
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
    std::string seq { *seq_buff };

    /*
     * UP: If not on the first line, cursor should move up 1 line,
     *     else, move to the previously ran command.
    */
    if (seq == "[A") {
        return true;
    }

    /*
     * DOWN: If not on the last line, cursor should move down 1 line,
     *       else, move to the next ran command, or do nothing.
    */
    if (seq == "[B") {
        return true;

    }

    /*
     * RIGHT: If not on the last character of the line, move cursor right once,
     *        else, move to the next line, and put cursor at
     *        the start of the line, else do nothing.
    */
    if (seq == "[C") {
        if (m_pos.x < utils::get_line(p_str, m_pos.y).length()) {
            io::print("\033[C");
            m_pos.x++;

            return true;
        }

        int64_t line_amount { std::ranges::count(p_str, '\n') };
        if (m_pos.y < line_amount) {
            io::print("\033[0G");
            io::print("\033[B");

            m_pos.x = 0;
            m_pos.y++;
        }

        return true;
    }

    /*
     * LEFT: If not on the first character of the line, move cursor left once,
     *       else, move to the previous line, and put cursor at
     *       the last character of the line, else do nothing.
    */
    if (seq == "[D") {
        if (m_pos.x > 0) {
            io::print("\033[D");
            m_pos.x--;

            return true;
        }

        if (m_pos.y > 0) {
            io::print("\033[A");
            m_pos.y--;

            m_pos.x = utils::get_line(p_str, m_pos.y).length() - 1;
            io::print("\033[{}G", m_pos.x + 1);
        }

        return true;
    }

    return true;
}


void
Handler::sigint_handler( int p_sig )
{
    if (p_sig == SIGINT && m_handler_instance) {
        if (!m_handler_instance->is_active()) return;

        io::println("^C");
        std::signal(SIGINT, Handler::sigint_handler);
    }
}