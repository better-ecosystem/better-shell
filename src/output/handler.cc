#include "output/handler.hh"

using output::Handler;


Handler::Handler(std::ostream *stream) :
    m_stream(stream)
{}


void
Handler::write(const std::string &text)
{
    m_stream->write(text.data(), text.length());
}