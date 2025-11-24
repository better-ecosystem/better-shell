#include <iostream>

#include <unistd.h>

#include "command/built_in.hh"
#include "input/handler.hh"
#include "print.hh"

using input::Handler;


Handler::Handler(int fd)
    : m_stream_fd(fd), m_exit(false), m_terminal_handler(fd)
{
}


auto
Handler::read(std::string &str) -> std::size_t
{
    str.clear();

    bool reading { true };
    char c;

    m_terminal_handler.show_prompt();

    while (reading)
    {
        if (term::Handler::is_sigint_triggered())
        {
            term::Handler::set_trigger_false();
            io::println("^C");
            str.clear();
            m_terminal_handler.reset();

            reading = false;
            continue;
        }

        ssize_t n { ::read(m_stream_fd, &c, 1) };

        if (n == 0)
        {
            this->exit(EOF);
            break;
        }

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            this->exit(EOF);
            break;
        }

        switch (m_terminal_handler.handle(c, str))
        {
        case term::RETURN_CONTINUE: continue;
        case term::RETURN_EXIT:     this->exit(c);
        case term::RETURN_DONE:     reading = false; continue;
        default:                    break;
        }

        if (!m_terminal_handler.is_active()) { str += c; }

        if (c == '\n') break;
    }

    if (m_terminal_handler.is_active()) m_terminal_handler.reset();
    return str.length();
}


auto
Handler::should_exit() const -> bool
{
    return m_exit || cmd::built_in::SHOULD_EXIT;
}


void
Handler::exit(char code)
{
    if (m_terminal_handler.is_active())
    {
        const char *type { code == EOT ? "EOT"
                                       : (code == EOF ? "EOF" : "EXIT") };
        io::println(std::cerr, "\n[{}]: {} ({})", type, APP_ID, APP_VERSION);
    }
    m_exit = true;
}
