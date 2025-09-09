#include <iostream>
#include "input/input.hh"
#include "print.hh"

using input::Handler;


Handler::Handler( std::istream *p_stream ) :
    m_stream(p_stream),
    m_exit(false),
    m_terminal_handler(p_stream)
{
}


auto
Handler::read( std::string &p_str ) -> size_t
{
    p_str.clear();
    std::streambuf *pbuf { m_stream->rdbuf() };

    BracketType bracket { BRACKET_NONE };
    bool escape { false };
    char c;

    m_terminal_handler.show_prompt();

    while (true) {
        /* Theres no EOF, because ICANON is disabled. */
        if (auto code { pbuf->sgetc() }; code == EOT || code == EOF) {
            this->exit();
            break;
        }

        c = pbuf->sbumpc();

        switch (m_terminal_handler.handle(c, p_str, pbuf)) {
        case term::RETURN_CONTINUE: continue;
        case term::RETURN_EXIT: this->exit();
        default: break;
        }

        if (!m_terminal_handler.is_active())
            p_str += c;

        if (c == '\n') break;
    }

    if (m_terminal_handler.is_active())
        m_terminal_handler.reset();

    return p_str.length();
}


auto
Handler::should_exit() -> bool
{ return m_exit; }


void
Handler::exit()
{
    io::println(std::cerr, "\n[EXIT]: {}", APP_NAME);
    m_exit = true;
}