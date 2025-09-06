#pragma once
#include <cstdint>
#include <istream>
#include <string>
#include <termios.h>

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
        ~Handler();


        /**
         * @brief Reads shell input from stream passed to the ctor.
         *
         * @return The size of @p p_str
         */
        auto read( std::string &p_str ) -> size_t;


        void exit();

    private:
        std::istream *m_stream;

        /* Might be uninitialized */
        bool    m_is_term;
        termios m_old_term;


        /**
         * @brief Handles arrow key inputs
         *
         * @param p_str    Buffer string passed to @e read .
         * @param p_cursor Cursor position.
         * @return true on success, false on EOT.
         */
        auto handle_arrows( const std::string &p_str,
                            size_t            &p_cursor,
                            std::streambuf    *p_sbuf ) -> bool;


        /**
         * @brief Handles backspace inputs
         *
         * @param p_str    Buffer string passed to @e read .
         * @param p_cursor Cursor position.
         */
        void handle_backspace( std::string &p_str,
                               size_t      &p_cursor );


        static Handler *m_handler_instance;


        static void sigint_handler( int p_sig );
    };
}