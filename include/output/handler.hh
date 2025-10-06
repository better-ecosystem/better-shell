#pragma once
#include <ostream>


namespace output
{
    /**
     * handles the shell output operation to output stream,
     * any output operation that needs to be done must be done
     * through this handler.
     */
    class Handler
    {
    public:
        /**
         * handle shell output to @p stream
         * the class does not own the stream
         */
        Handler(std::ostream *stream);


        /**
         * writes a string @p text
         */
        void write(const std::string &text);

    private:
        std::ostream *m_stream;
    };
}