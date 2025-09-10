#pragma once
#ifndef __BetterShell__input_terminal_hh
#define __BetterShell__input_terminal_hh
#include <cstdint>
#include <istream>
#include <termios.h>
#include "input/cursor_pos.hh"


namespace input::term
{
#define EOT 4

    enum ReturnType : uint8_t
    {
        RETURN_CONTINUE,
        RETURN_NONE,
        RETURN_DONE,
        RETURN_EXIT
    };


    class Handler
    {
    public:
        Handler( std::istream *p_stream );
       ~Handler();


       auto handle( const char     &p_current,
                    std::string    &p_str,
                    std::streambuf *p_sbuf ) -> ReturnType;


       /**
        * @brief Resets the cursor position.
        */
       void reset();


       void show_prompt();


       [[nodiscard]]
       auto is_active() -> bool;

    private:
        std::istream *m_stream;

        CursorPosition m_pos;
        bool m_escaped;

        termios m_old_term;
        bool    m_is_term;


        void insert_char_to_cursor( std::string &p_str, char p_c );


        void handle_backspace( std::string &p_str, bool p_ctrl );
        auto handle_arrow( const std::string &p_str,
                           std::streambuf    *p_sbuf ) -> bool;

        auto handle_history( const std::string &p_current ) -> bool;

        static Handler *m_handler_instance;
        static void sigint_handler( int p_sig );
    };
}

#endif /* __BetterShell__input_terminal_hh */