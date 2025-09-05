#pragma once
#include <istream>
#include <string>
#include <termios.h>


namespace input
{
    class Handler
    {
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

    private:
        std::istream *m_stream;

        /* Might be uninitialized */
        bool    m_is_term;
        termios m_old_term;
    };
}