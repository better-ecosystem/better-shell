#pragma once
#ifndef __BetterShell__input_terminal_hh
#define __BetterShell__input_terminal_hh
#include <cstdint>
#include <istream>
#include <termios.h>


namespace input::term
{
#define EOT 4


    struct CursorPosition
    {
        uint32_t x { 0 };
        uint32_t y { 0 };

        uint32_t last_x { 0 };


        [[nodiscard]]
        auto
        is_zero() const noexcept -> bool
        { return x == 0 && y == 0; }
    };

    enum ReturnType : uint8_t
    {
        RETURN_CONTINUE,
        RETURN_NONE,
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

        termios m_old_term;
        bool    m_is_term;


        void insert_char_to_cursor( std::string &p_str, char p_c );


        void handle_backspace( std::string &p_str, bool p_ctrl );
        auto handle_arrow( const std::string &p_str,
                           std::streambuf    *p_sbuf ) -> bool;


        static Handler *m_handler_instance;
        static void sigint_handler( int p_sig );
    };
}

#endif /* __BetterShell__input_terminal_hh */