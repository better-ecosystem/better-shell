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
    bool reading { true };
    char c;

    input::term::Handler::show_prompt();

    while (reading) {
        /* Theres no EOF, because ICANON is disabled. */
        if (auto code { pbuf->sgetc() }; code == EOT || code == EOF) {
            this->exit(static_cast<char>(code));
            break;
        }

        c = static_cast<char>(pbuf->sbumpc());

        switch (m_terminal_handler.handle(c, p_str, pbuf)) {
        case term::RETURN_CONTINUE: continue;
        case term::RETURN_EXIT: this->exit(c);
        case term::RETURN_DONE: reading = false; continue;
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
Handler::should_exit() const -> bool
{ return m_exit; }


void
Handler::exit( char p_code )
{
    const char *type { p_code == EOT ? "EOT" :
                      (p_code == EOF ? "EOF" : "EXIT") };
    io::println(std::cerr, "\n[{}]: {} ({})",
                type, APP_ID, APP_VERSION);
    m_exit = true;
}