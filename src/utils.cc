#include <algorithm>
#include <format>

#include "utils.hh"


namespace
{
    [[nodiscard]]
    auto
    is_valid_utf8(std::string_view str) -> bool
    {
        int expected { 0 };

        for (auto c : str)
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

        return expected == 0;
    }
} /* anonymous namespace */


namespace utils
{
    auto
    get_line(const std::string &str, size_t idx) -> std::string
    {
        size_t line_start { 0 };
        size_t line_no { 0 };

        for (size_t i { 0 }; i <= str.size(); i++)
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
        return std::ranges::any_of(str, [](unsigned char c) -> int
                                   { return c & 0x80; });
    }


    auto
    getenv(const std::string &env) -> std::string
    {
        const auto *VALUE { std::getenv(env.c_str()) };
        return VALUE == nullptr ? "" : VALUE;
    }


    auto
    getenv(const std::string &env, const std::string &val) -> std::string
    {
        const auto *VALUE { std::getenv(env.c_str()) };
        return VALUE == nullptr ? val : VALUE;
    }
} /* namespace utils */
