#include <algorithm>
#include <format>

#include "utils.hh"


namespace utils::str
{
    auto
    split(const std::string &str, char delim) -> std::vector<std::string>
    {
        std::vector<std::string> lines;
        lines.reserve(std::ranges::count(str, delim) + 1);

        std::istringstream iss { str };
        for (std::string line; std::getline(iss, line, delim);)
            lines.emplace_back(line);

        return lines;
    }


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
    is_word_bound(const unsigned char &ch) -> bool
    {
        return (std::isspace(ch) != 0) || (std::ispunct(ch) != 0);
    }


    auto
    move_idx_to_direction(const std::string &str, size_t index, int direction)
        -> size_t
    {
        const size_t LEN { str.length() };

        if (direction == -1 && index > 0)
        {
            bool is_bound { utils::str::is_word_bound(str[index - 1]) };

            while (index > 0
                   && utils::str::is_word_bound(str[index - 1]) == is_bound)
            {
                index--;
            }
        }
        else if (direction == 1 && index < LEN)
        {
            bool is_bound { utils::str::is_word_bound(str[index]) };
            while (index < LEN
                   && utils::str::is_word_bound(str[index]) == is_bound)
            {
                index++;
            }
        }

        return index;
    }


    auto
    trim(const std::string &str) -> std::string
    {
        const auto begin { str.find_first_not_of(" \t\n\r\f\v") };
        if (begin == std::string::npos) return "";

        const auto end { str.find_last_not_of(" \t\n\r\f\v") };
        return str.substr(begin, end - begin + 1);
    }


    auto
    split(const std::string &str, size_t pos)
        -> std::pair<std::string, std::string>
    {
        if (pos == std::string::npos) return { str, "" };

        std::string first { str.substr(0, pos) };
        std::string second { str.substr(pos + 1) };
        return { first, second };
    }


    auto
    is_empty(const std::string &str) -> bool
    {
        if (str.empty()) return true;
        return std::ranges::all_of(str, [](const char &ch) -> bool
                                   { return std::isspace(ch) != 0; });
    }
}
