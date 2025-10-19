#pragma once
#include <cstdint>
#include <istream>

#include <termios.h>

#include "history/history.hh"
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


        void reset();


        void show_prompt();


        [[nodiscard]]
        auto is_active() const -> bool;

    private:
        static Handler *m_handler_instance;

        std::unique_ptr<history::Handler> m_history;
        std::string                       m_current_text;

        std::istream *m_stream;
        std::string   m_u8_buffer;
        size_t        m_u8_expected_len;

        std::string m_prompt;

        Cursor m_pos;
        bool   m_escaped;

        size_t m_highlight_start_pos;

        termios m_old_term;
        bool    m_is_term;


        void insert_char_to_cursor(std::string &str, unsigned char c);


        void insert_u8_to_cursor(std::string &str);


        void handle_backspace(std::string &str, bool ctrl);


        void handle_delete(std::string &str, bool ctrl);


        auto handle_highlight(const std::string &str, const std::string &seq)
            -> bool;


        auto handle_ansi(std::string &str, std::streambuf *sbuf) -> bool;


        auto handle_history(Cursor::Direction direction,
                            std::string      &current_text) -> bool;

        static void sigint_handler(int sig);
    };
}
