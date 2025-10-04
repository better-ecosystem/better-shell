#pragma once
#ifndef __BetterShell__parser_handler_hh
#define __BetterShell__parser_handler_hh
#include <string>
#include <thread>


namespace parser
{
    class Handler
    {
    public:
        Handler( bool syntax_highlighting );


        void parse_token( const std::string &str );


        [[nodiscard]]
        auto get_echo() const -> const std::string;

    private:
        const bool m_syntax_highlighting;
        std::thread m_thread;

    };
}

#endif /* __BetterShell__parser_handler_hh */