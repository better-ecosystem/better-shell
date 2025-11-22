#pragma once
#include <istream>
#include <string>

#include <termios.h>

#include "input/terminal.hh"

#define EOT 4


namespace input
{
    class Handler
    {
    public:
        /**
         * handle shell input from @p stream
         * the class does not own the stream
         */
        Handler(std::istream *stream);


        /**
         * reads a character and returns the size of the string
         */
        auto read(std::string &str) -> std::size_t;


        /**
         * checks whether the shell is supposed to exit or not
         */
        [[nodiscard]]
        auto should_exit() const -> bool;


    private:
        std::istream *m_stream;
        bool          m_exit;

        term::Handler m_terminal_handler;


        /**
         * force exit the shell with code @p code (can be EOF or EOT)
         */
        void exit(char code);
    };
}
