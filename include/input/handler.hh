#pragma once
#ifndef __BetterShell__input_handler_hh
#define __BetterShell__input_handler_hh
#include <cstdint>
#include <istream>
#include <string>
#include <termios.h>
#include "parser/handler.hh"
#include "input/terminal.hh"

#define EOT 4


namespace input
{
    class Handler
    {
        enum BracketType : uint8_t
        {
            BRACKET_SINGLE,
            BRACKET_DOUBLE,
            BRACKET_NONE,
        };

    public:
        /**
         * @param stream The stream the handler will read into.
         * @warning deleting @p stream while this class is still active
         *          is undefined behaviour.
         */
        Handler( std::istream *stream );


        /**
         * @brief Reads shell input from stream passed to the ctor.
         *
         * @return The size of @p str
         */
        auto read( std::string &str ) -> size_t;


        /**
         * @return true if the shell is supposed to exit, false otherwise.
         */
        [[nodiscard]]
        auto should_exit() const -> bool;


    private:
        std::istream *m_stream;
        bool m_exit;

        term::Handler m_terminal_handler;
        // parser::Handler m_parser;


        /**
         * @brief Exits the shell.
         */
        void exit( char code );
    };
}

#endif /* __BetterShell__input_handler_hh */