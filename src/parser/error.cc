#include "parser/error.hh"

using parser::Error;


auto
Error::get_message() const -> std::string
{
    return m_pretty;
}


auto
Error::line_column_from_offset(const std::string &text, size_t offset)
    -> std::pair<size_t, size_t>
{
    size_t line { 0 };
    size_t col { 0 };

    for (size_t i { 0 }; i < offset && i < text.size(); i++)
    {
        if (text[i] == '\n')
        {
            line++;
            col = 0;
        }
        else
            col++;
    }

    return { line, col };
}