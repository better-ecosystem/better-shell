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
    } /* namespace str */


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
