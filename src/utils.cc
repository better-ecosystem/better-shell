#include <algorithm>
#include <format>
#include "utils.hh"


namespace
{
    auto
    is_word_char( char p_c ) -> bool
    { return (std::isalnum(p_c) != 0) || p_c == '_'; }


    [[nodiscard]]
    auto
    is_valid_utf8( std::string_view p_str ) -> bool
    {
        int expected { 0 };

        for (unsigned char c : p_str) {
            if (expected == 0) {
                if      ((c >> 5) == 0b110)   expected = 1;
                else if ((c >> 4) == 0b1110)  expected = 2;
                else if ((c >> 3) == 0b11110) expected = 3;
                else if ((c >> 7) != 0) return false;
            } else {
                if ((c >> 6) != 0b10) return false;
                expected--;
            }
        }
        return expected == 0;
    }
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
            while (i < n && std::isspace(p_str[i]) != 0) i++;
            if (i >= n) return i - p_pos;

            bool in_word { is_word_char(p_str[i]) };

            if (in_word) while (i < n && is_word_char(p_str[i])) i++;
            else while (i < n && !is_word_char(p_str[i])
                              && std::isspace(p_str[i]) != 0) i++;

            return i - p_pos;
        }

        /* Left */
        if (i == 0) return 0;
        size_t start { i };

        while (i > 0 && std::isspace(p_str[i - 1]) != 0) i--;
        if (i == 0) return -(start - i);

        bool in_word { is_word_char(p_str[i - 1]) };

        if (in_word) while (i > 0 && is_word_char(p_str[i - 1])) i--;
        else while (i > 0 && !is_word_char(p_str[i - 1])
                          && std::isspace(p_str[i - 1]) != 0) i--;

        return -(start - i);
    }


    auto
    get_line( const std::string &p_str, size_t p_idx ) -> std::string
    {
        size_t line_start { 0 };
        size_t line_no    { 0 };

        for (size_t i = 0; i <= p_str.size(); i++)
            if (i == p_str.size() || p_str[i] == '\n') {
                if (line_no == p_idx)
                    return p_str.substr(line_start, i - line_start);
                line_no++;
                line_start = i + 1;
            }

        throw std::out_of_range(
            std::format("p_idx ({}) is larger than the "
                        "number of lines in p_str.", p_idx));
    }


    auto
    contains_unicode( std::string_view p_str ) -> bool
    {
        if (!is_valid_utf8(p_str)) return false;
        return std::ranges::any_of(p_str,
               []( unsigned char p_c ){ return p_c & 0x80; });
    }
}