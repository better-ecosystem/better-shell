#include <iostream>
#include <csignal>
#include <format>
#include <array>
#include <unistd.h>
#include "input/input.hh"

using input::Handler;


Handler *Handler::m_handler_instance { nullptr };


Handler::Handler( std::istream *p_stream ) :
    m_stream(p_stream)
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
Handler::read( std::string &p_str ) -> size_t
{
    p_str.clear();
    std::streambuf *pbuf { m_stream->rdbuf() };

    bool reading { true };
    BracketType bracket { BRACKET_NONE };
    char c;
    size_t cursor { 0 };

    while (reading) {
        /* Theres no EOF, because ICANON is disabled. */
        if (pbuf->sgetc() == EOT)
            this->exit();

        c = pbuf->sbumpc();
        if (m_is_term) {
            std::cerr << c;
            cursor++;
        }

        if (c == '\x1b') {
            if (!handle_arrows(p_str, cursor, pbuf))
                this->exit();
            continue;
        }

        /* Backspace */
        if (c == 0x7f || c == 0x08) {
            handle_backspace(p_str, cursor);
            continue;
        }


        if (c == '\n') {
            p_str += c;
            reading = false;
        } else {
            p_str += c;
        }
    }

    return p_str.length();
}


void
Handler::exit()
{
    std::cerr << "[EXIT]: " << APP_NAME << '\n';
    std::exit(0);
}


auto
Handler::handle_arrows( const std::string &p_str,
                        size_t            &p_cursor,
                        std::streambuf    *p_sbuf ) -> bool
{
    std::array<char, 8> buff;
    for (uint16_t i { 0 }; i < buff.size(); i++) {
        if (p_sbuf->sgetc() == EOT) return false;

        buff[i] = p_sbuf->sbumpc();
        if ((buff[i] >= 'A' && buff[i] <= 'Z') || buff[i] == '~') {
            buff[++i] = '\0';
            break;
        }
    }

    std::string seq { buff.data() };
    if      (seq == "[A"   ) {}
    else if (seq == "[B"   ) {}
    else if (seq == "[C"   ) {
        std::cerr << "\033[C";
        p_cursor++;
    }
    else if (seq == "[D"   ) {
        std::cerr << "\033[D";
        p_cursor--;
    }
    else if (seq == "[1;5A") {}
    else if (seq == "[1;5B") {}
    else if (seq == "[1;5C") {
        
    }
    else if (seq == "[1;5D") std::cerr << "<CTRL+LEFT>";
    else std::cout << "<ESC " << seq << ">";

    return true;
}


void
Handler::handle_backspace( std::string &p_str,
                           size_t      &p_cursor )
{
    if (p_cursor > 0) {
        p_str.erase(p_cursor - 1, 1);
        p_cursor--;

        if (!m_is_term) return;

        uint32_t rm_str { 1 };
        if (p_cursor + 1 < p_str.length()) {
            std::string right { p_str.substr(p_cursor + 1) };
            rm_str = right.length() + 1;
        }

        std::cerr << std::format("\033[{}D", rm_str);
        std::cerr << "\033[s";
        std::cerr << p_str.substr(p_cursor);
        std::cerr << ' ';
        std::cerr << "\033[u";
    }
}


void
Handler::sigint_handler( int p_sig )
{
    if (p_sig == SIGINT && m_handler_instance) {
        std::cerr << "^C\n";
        std::signal(SIGINT, Handler::sigint_handler);
    }
}