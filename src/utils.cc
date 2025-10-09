#include <algorithm>
#include <format>

#include <json/reader.h>
#include <json/writer.h>

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
    namespace utf8
    {
        auto
        contains_unicode(std::string_view str) -> bool
        {
            if (!is_valid_utf8(str)) return false;
            return std::ranges::any_of(str, [](unsigned char c) -> int
                                       { return c & 0x80; });
        }


        auto
        is_leading_byte(const unsigned char &byte) -> bool
        {
            return (byte & 0b11000000) == 0b11000000
                && (byte & 0b11111000) != 0b11111000;
        }


        auto
        is_continuation_byte(const unsigned char &byte) -> bool
        {
            return (byte & 0b11000000) == 0b10000000;
        }


        auto
        is_ascii_byte(const unsigned char &byte) -> bool
        {
            return (byte & 0b10000000) == 0;
        }


        auto
        get_expected_length(const unsigned char &leading) -> size_t
        {
            if ((leading & 0b10000000) == 0) return 1;
            if ((leading & 0b11100000) == 0b11000000) return 2;
            if ((leading & 0b11110000) == 0b11100000) return 3;
            if ((leading & 0b11111000) == 0b11110000) return 4;
            return 1;
        }


        auto
        length(const std::string &str) -> size_t
        {
            size_t length { 0 };
            size_t i { 0 };

            while (i < str.size())
            {
                auto c { static_cast<unsigned char>(str[i]) };

                size_t char_len { get_expected_length(c) };

                i += char_len;
                if (i > str.size())
                    throw std::runtime_error("Truncated UTF-8 character");

                length++;
            }

            return length;
        }
    } /* namespace utf8 */


    namespace str
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
        move_idx_to_direction(const std::string &str,
                              size_t             index,
                              int                direction) -> size_t
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
    } /* namespace str */


    namespace ansi
    {
        auto
        is_arrow(const std::string &seq) -> bool
        {
            const char BACK { seq.back() };
            return BACK >= 'A' && BACK <= 'D';
        }


        auto
        is_ctrl_pressed(const std::string &seq) -> bool
        {
            size_t idx { seq.find_first_of(';') };
            if (idx == std::string::npos) return false;
            return seq.at(idx + 1) == '5' || seq.at(idx + 1) == '6';
        }


        auto
        is_shift_pressed(const std::string &seq) -> bool
        {
            size_t idx { seq.find_first_of(';') };
            if (idx == std::string::npos) return false;
            return seq.at(idx + 1) == '2' || seq.at(idx + 1) == '6';
        }


        auto
        is_home_end(const std::string &seq) -> int
        {
            const char BACK { seq.back() };

            if (BACK == 'H') return -1;
            if (BACK == 'F') return 1;
            if (BACK == '~')
            {
                if (seq[1] == '1') return -1;
                if (seq[1] == '4') return 1;
            }

            return 0;
        }


        auto
        get_ansi_color(const std::string &seq) -> std::array<int, 3>
        {
            auto parts { utils::str::split(seq.substr(1, seq.length() - 2),
                                           ';') };

            /* 255 or RGB */
            if (parts.size() >= 3)
            {
                /* 255 */
                if (parts[1] == "5")
                {
                    if (parts.size() != 3) return INVALID_COLOR;

                    int value { std::stoi(parts[2]) };
                    if (value < 8) return COLORS[value];
                    if (value < 16) return COLORS[value - 8];
                    if (value > 231)
                    {
                        int gray { 8 + ((value - 232) * 10) };
                        return { gray, gray, gray };
                    }

                    /* 6 × 6 × 6 cube */
                    auto comp { [](int v) -> int
                                { return v == 0 ? 0 : 55 + (v * 40); } };

                    int index { value - 16 };
                    int r { index / 36 };
                    int g { (index / 6) % 6 };
                    int b { index % 6 };

                    return { comp(r), comp(g), comp(b) };
                }

                /* RGB */
                if (parts[1] == "2")
                {
                    return { std::stoi(parts[2]), std::stoi(parts[3]),
                             std::stoi(parts[4]) };
                }

                return INVALID_COLOR;
            }

            /* 30 - 37 */
            int code { std::clamp(std::stoi(parts.back()) - 30, 0, 7) };
            return COLORS[code];
        }


        auto
        get_highlighted_foreground(const std::string &ansi_seq) -> bool
        {
            std::array<int, 3> rgb { get_ansi_color(ansi_seq) };

            double brightness { (rgb[0] * 299 + rgb[1] * 587 + rgb[2] * 114)
                                / 1000.0F };

            return brightness <= 128;
        }
    } /* namespace ansi */


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


namespace Json
{
    auto
    to_string(const Json::Value &val) -> std::string
    {
        StreamWriterBuilder builder;
        builder["indentation"]             = "    ";
        builder["enableYAMLCompatibility"] = true;
        return writeString(builder, val);
    }


    auto
    from_string(const std::string &val) -> Json::Value
    {
        CharReaderBuilder builder;
        builder["collectComments"] = false;

        std::string        errs;
        std::istringstream iss { val };
        Json::Value        root;
        parseFromStream(builder, iss, &root, &errs);
        return root;
    }
}
