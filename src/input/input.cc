#include <iostream>
#include <csignal>
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


auto
Handler::read( std::string &p_str ) -> size_t
{
    p_str.clear();
    std::streambuf *pbuf { m_stream->rdbuf() };

    bool reading { true };
    char c;

    while (reading) {
        /* Theres no EOF, because ICANON is disabled. */
        if (pbuf->sgetc() == 4) {
            std::cerr << "[EXIT]: " << APP_NAME << '\n';
            std::exit(0);
        }

        c = pbuf->sbumpc();

        if (m_is_term) std::cout << c;

        if (c == '\n') {
            p_str += c;
            reading = false;
        } else {
            p_str += c;
        }
    }

    return p_str.length();
}


Handler::~Handler()
{ if (m_is_term) tcsetattr(STDIN_FILENO, TCSANOW, &m_old_term); }


void
Handler::sigint_handler( int p_sig )
{
    if (p_sig == SIGINT && m_handler_instance) {
        std::cerr << "^C\n";
        std::signal(SIGINT, Handler::sigint_handler);
    }
}