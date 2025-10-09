#include <algorithm>

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


    auto
    levenshtein_distance(const std::string &a, const std::string &b) -> int
    {
        const std::spair<size_t>      LEN { a.length(), b.length() };
        std::vector<std::vector<int>> dp { LEN.first + 1,
                                           std::vector<int>(LEN.second + 1) };

        for (size_t i { 0 }; i <= LEN.first; i++) dp[i][0] = i;
        for (size_t j { 0 }; j <= LEN.second; j++) dp[0][j] = j;

        for (size_t i { 1 }; i <= LEN.first; i++)
        {
            for (size_t j { 1 }; j <= LEN.second; j++)
            {
                int cost { (a[i - 1] == b[j - 1]) ? 0 : 1 };
                dp[i][j] = std::min({ dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                                      dp[i - 1][j - 1] + cost });
            }
        }

        return dp[LEN.first][LEN.second];
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
