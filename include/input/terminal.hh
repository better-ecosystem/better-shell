#pragma once
#include <cstdint>
#include <istream>

#include <termios.h>

#include "input/cursor.hh"


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
        Handler(std::istream *stream);
        ~Handler();


        auto handle(const unsigned char &current,
                    std::string         &str,
                    std::streambuf      *sbuf) -> ReturnType;


        /**
        * @brief Resets the cursor position.
        */
        void reset();


        static void show_prompt();


        [[nodiscard]]
        auto is_active() const -> bool;

    private:
        std::istream *m_stream;
        std::string   m_u8_buffer;
        size_t        m_u8_expected_len;

        Cursor m_pos;
        bool   m_escaped;

        termios m_old_term;
        bool    m_is_term;


        void insert_char_to_cursor(std::string &str, unsigned char c);
        void insert_u8_to_cursor(std::string &str);


        void handle_backspace(std::string &str, bool ctrl);
        auto handle_arrow(const std::string &str, std::streambuf *sbuf) -> bool;

        auto handle_history(const std::string &current) -> bool;

        static Handler *m_handler_instance;
        static void     sigint_handler(int sig);
    };
}
