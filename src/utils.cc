#include <algorithm>
#include <format>

#include "utils.hh"


namespace
{
    auto
    is_word_char(char c) -> bool
    {
        return (std::isalnum(c) != 0) || c == '_';
    }


    [[nodiscard]]
    auto
    is_valid_utf8(std::string_view str) -> bool
    {
        int expected { 0 };

        for (unsigned char c : str)
        {
            if (expected == 0)
            {
                if ((c >> 5) == 0b110)
                    expected = 1;
                else if ((c >> 4) == 0b1110)
                    expected = 2;
                else if ((c >> 3) == 0b11110)
                    expected = 3;
                else if ((c >> 7) != 0)
                    return false;
            }
            else
            {
                if ((c >> 6) != 0b10) return false;
                expected--;
            }
        }
        return expected == 0;
    }
} /* anonymous namespace */


namespace utils
{
    auto
    get_cursor_word_jump_offset(const std::string &str,
                                const size_t      &pos,
                                const char        &direction) -> size_t
    {
        if (str.empty()) return 0;

        size_t n { str.size() };
        size_t i { pos };

        /* Right */
        if (direction == 'D')
        {
            if (i >= n) return 0;
            while (i < n && std::isspace(str[i]) != 0) i++;
            if (i >= n) return i - pos;

            bool in_word { is_word_char(str[i]) };

            if (in_word)
                while (i < n && is_word_char(str[i])) i++;
            else
                while (i < n && !is_word_char(str[i])
                       && std::isspace(str[i]) != 0)
                    i++;

            return i - pos;
        }

        /* Left */
        if (i == 0) return 0;
        size_t start { i };

        while (i > 0 && std::isspace(str[i - 1]) != 0) i--;
        if (i == 0) return -(start - i);

        bool in_word { is_word_char(str[i - 1]) };

        if (in_word)
            while (i > 0 && is_word_char(str[i - 1])) i--;
        else
            while (i > 0 && !is_word_char(str[i - 1])
                   && std::isspace(str[i - 1]) != 0)
                i--;

        return -(start - i);
    }


    auto
    get_line(const std::string &str, size_t idx) -> std::string
    {
        size_t line_start { 0 };
        size_t line_no { 0 };

        for (size_t i = 0; i <= str.size(); i++)
            if (i == str.size() || str[i] == '\n')
            {
                if (line_no == idx)
                    return str.substr(line_start, i - line_start);
                line_no++;
                line_start = i + 1;
            }

        throw std::out_of_range(std::format("idx ({}) is larger than the "
                                            "number of lines in str.",
                                            idx));
    }


    auto
    contains_unicode(std::string_view str) -> bool
    {
        if (!is_valid_utf8(str)) return false;
        return std::ranges::any_of(str, [](unsigned char c)
                                   { return c & 0x80; });
    }
} /* namespace utils */
