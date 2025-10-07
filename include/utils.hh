#pragma once
#include <ranges>
#include <string>

#include <json/value.h>


namespace utils
{
    namespace utf8
    {
        /**
         * checks if @p str contains any non-ASCII characters
         */
        [[nodiscard]]
        auto contains_unicode(std::string_view str) -> bool;


        /**
         * checks if a byte @p byte is the leading byte of
         * a UTF-8 multi-byte sequence
         */
        [[nodiscard]]
        auto is_leading_byte(const unsigned char &byte) -> bool;


        /**
         * checks if a byte @p byte is a UTF-8 continuation byte
         */
        [[nodiscard]]
        auto is_continuation_byte(const unsigned char &byte) -> bool;


        /**
         * checks if a byte @p byte is an ASCII byte
         */
        [[nodiscard]]
        auto is_ascii_byte(const unsigned char &byte) -> bool;


        /**
         * gets the expected length of a UTF-8 multi-byte sequence based on the
         * leading byte
         */
        [[nodiscard]]
        auto get_expected_length(const unsigned char &leading) -> size_t;


        /**
         * this function is an std::string::length() wrapper that is UTF-8 aware
         */
        [[nodiscard]]
        auto length(const std::string &str) -> size_t;
    }


    namespace str
    {
        /**
         * transform a multi-line string @p str into a vector of lines
         */
        [[nodiscard]]
        auto split_lines(const std::string &str) -> std::vector<std::string>;


        /**
         * returns the line at index @p idx from @p str
         * --------------------------------------------
         *
         * this function may throw an std::out_of_range if @p idx exceeds
         * the number of lines inside @p str
         */
        [[nodiscard]]
        auto get_line(const std::string &str, size_t idx) -> std::string;


        /**
         * checks if @p ch is a word bound
         */
        [[nodiscard]]
        auto is_word_bound(const unsigned char &ch) -> bool;


        /**
         * moves @p idx one word to @p direction
         * -------------------------------------
         *
         * @p direction can be -1 for left, or 1 for right
         */
        [[nodiscard]]
        auto move_idx_to_direction(const std::string &str,
                                   size_t             index,
                                   int                direction) -> size_t;


        /**
         * trims excess whitespace from @p str
         */
        [[nodiscard]]
        auto trim(const std::string &str) -> std::string;
    }


    /**
     * ANSI sequence utility namespace
     * -------------------------------
     *
     * all functions in this namespace that request a @p seq
     * should be given the sequence starting from the open
     * square bracket '['. (e.g., "[A", "[1;5~")
     */
    namespace ansi
    {
        /**
         * checks whether @p seq represents an arrow key
         */
        [[nodiscard]]
        auto is_arrow(const std::string &seq) -> bool;


        /**
         * checks if Ctrl was held during a sequence @p seq
         */
        [[nodiscard]]
        auto is_ctrl_pressed(const std::string &seq) -> bool;


        /**
         * determines if @p seq is a Home or End key
         * -----------------------------------------
         *
         * the function will return
         *
         *  -1 if @p seq is a Home key
         *
         *   1 if @p seq is an End key
         *
         *   0 if @p seq is neither
         */
        [[nodiscard]]
        auto is_home_end(const std::string &seq) -> int;


#ifndef BETTER_SH_NO_COLOR

#define COLOR_RESET "\033[0;0;0m"
#define ANSI_RGB_FG(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define ANSI_RGB_BG(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"

#else

#define COLOR_RESET ""
#define ANSI_RGB_FG(r, g, b) ""
#define ANSI_RGB_BG(r, g, b) ""

#endif
    }


    /**
     * fetch an environment variable @p env , returns a string of the value
     * or an empty string if the variable does not exist
     */
    [[nodiscard]]
    auto getenv(const std::string &env) -> std::string;


    /**
     * fetch an environment variable @p env , returns a string of the value
     * or @p val if the variable does not exist
     */
    [[nodiscard]]
    auto getenv(const std::string &env, const std::string &val) -> std::string;


    /**
     * creates a lazy integer range from
     * @p start (inclusive) to @p end (exclusive)
     */
    template <typename Tp>
    [[nodiscard]]
    constexpr auto
    range(Tp start, Tp end)
    {
        return std::views::iota(start) | std::views::take(end);
    }


    /**
     * repeats function @p n times
     */
#define RUN_FUNC_N(n, func, ...)                                    \
    for (auto _ : utils::range(static_cast<decltype((n))>(0), (n))) \
        func(__VA_ARGS__);

#define MAXB(a, b) std::max<decltype(b)>(a, b)
#define MAXA(a, b) std::max<decltype(a)>(a, b)
}


namespace Json
{
    [[nodiscard]]
    auto to_string(const Json::Value &val) -> std::string;


    [[nodiscard]]
    auto from_string(const std::string &val) -> Json::Value;
}
