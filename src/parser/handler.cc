#include "parser/handler.hh"

using parser::Handler;


Handler::Handler( bool syntax_highlighting ) :
    m_syntax_highlighting(syntax_highlighting)
{}


void
Handler::parse_token( const std::string &str )
{
    m_thread.join();
    
}