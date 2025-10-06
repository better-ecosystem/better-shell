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


#define CHECK direction < 0 ? index > 0 : index < LEN
        auto
        move_idx_to_direction(const std::string &str,
                              size_t             index,
                              int                direction) -> size_t
        {
            const int    OFFSET { direction < 0 ? 1 : 0 };
            const size_t LEN { str.length() };

            if (CHECK && utils::str::is_word_bound(index - OFFSET))
            {
                index += direction;
                while (CHECK && utils::str::is_word_bound(index - OFFSET))
                    index += direction;
            }
            else
            {
                while (CHECK && !utils::str::is_word_bound(index - OFFSET))
                    index += direction;
            }

            return index;
        }
#undef CHECK
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
            return seq.at(idx + 1) == 5;
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
        builder["indentation"] = "";
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
