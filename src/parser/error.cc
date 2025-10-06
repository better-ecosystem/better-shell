#include "parser/error.hh"

using parser::Error;


auto
Error::get_message() const -> std::string
{
    return m_pretty;
}
