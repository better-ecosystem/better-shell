#include <iostream>
#include <csignal>
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

    while (reading) {
        /* Theres no EOF, because ICANON is disabled. */
        if (pbuf->sgetc() == EOT)
            this->exit();

        c = pbuf->sbumpc();
        if (m_is_term) std::cerr << c;

        if (c == '\x1b') {
            if (!handle_arrows(p_str, pbuf))
                this->exit();
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
    else if (seq == "[C"   ) std::cerr << "\033[C";
    else if (seq == "[D"   ) std::cerr << "\033[D";
    else if (seq == "[1;5A") {}
    else if (seq == "[1;5B") {}
    else if (seq == "[1;5C") std::cerr << "<CTRL+RIGHT>";
    else if (seq == "[1;5D") std::cerr << "<CTRL+LEFT>";
    else std::cout << "<ESC " << seq << ">";

    return true;
}


void
Handler::sigint_handler( int p_sig )
{
    if (p_sig == SIGINT && m_handler_instance) {
        std::cerr << "^C\n";
        std::signal(SIGINT, Handler::sigint_handler);
    }
}