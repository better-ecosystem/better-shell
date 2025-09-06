#include "utils.hh"


namespace
{
    auto
    is_word_char( char c ) -> bool
    { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }
}


namespace utils
{
    auto
    get_cursor_word_jump_offset( const std::string &p_str,
                                 const size_t      &p_pos,
                                 const char        &p_direction ) -> size_t
    {
        if (p_str.empty()) return 0;

        size_t n { p_str.size() };
        size_t i { p_pos };

        /* Right */
        if (p_direction == 'D') {
            if (i >= n) return 0;
            while (i < n && std::isspace(p_str[i])) i++;
            if (i >= n) return i - p_pos;

            bool in_word { is_word_char(p_str[i]) };

            if (in_word) while (i < n && is_word_char(p_str[i])) i++;
            else while (i < n && !is_word_char(p_str[i])
                              && !std::isspace(p_str[i])) i++;

            return i - p_pos;
        }

        /* Left */
        if (i == 0) return 0;
        size_t start { i };

        while (i > 0 && std::isspace(p_str[i - 1])) i--;
        if (i == 0) return -(start - i);

        bool in_word { is_word_char(p_str[i - 1]) };

        if (in_word) while (i > 0 && is_word_char(p_str[i - 1])) i--;
        else while (i > 0 && !is_word_char(p_str[i - 1])
                          && !std::isspace(p_str[i - 1])) i--;

        return -(start - i);
    }
}