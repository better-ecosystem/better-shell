#include "parser/handler.hh"

using parser::Handler;


Handler::Handler( bool p_syntax_highlighting ) :
    m_syntax_highlighting(p_syntax_highlighting)
{}


void
Handler::parse_token( const std::string &p_str )
{
    m_thread.join();
    
}