#pragma once
#ifndef __BetterShell__input_input_hh
#define __BetterShell__input_input_hh
#include <cstdint>
#include <istream>
#include <string>
#include <termios.h>
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
         * @param p_stream The stream the handler will read into.
         * @warning deleting @p p_stream while this class is still active
         *          is undefined behaviour.
         */
        Handler( std::istream *p_stream );


        /**
         * @brief Reads shell input from stream passed to the ctor.
         *
         * @return The size of @p p_str
         */
        auto read( std::string &p_str ) -> size_t;


        /**
         * @return true if the shell is supposed to exit, false otherwise.
         */
        [[nodiscard]]
        auto should_exit() -> bool;


    private:
        std::istream *m_stream;
        bool m_exit;

        term::Handler m_terminal_handler;


        /**
         * @brief Exits the shell.
         */
        void exit();
    };
}

#endif /* __BetterShell__input_input_hh */