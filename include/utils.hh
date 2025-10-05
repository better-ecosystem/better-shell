#pragma once
#include <ranges>
#include <string>

#include <json/value.h>


namespace utils
{
    namespace utf8
    {
        [[nodiscard]]
        auto contains_unicode(std::string_view str) -> bool;


        [[nodiscard]]
        auto is_leading_byte(const unsigned char &byte) -> bool;


        [[nodiscard]]
        auto is_continuation_byte(const unsigned char &byte) -> bool;


        [[nodiscard]]
        auto is_ascii_byte(const unsigned char &byte) -> bool;


        [[nodiscard]]
        auto get_expected_length(const unsigned char &leading) -> size_t;


        [[nodiscard]]
        auto length(const std::string &str) -> size_t;
    }


    namespace str
    {
        /**
         * @brief Gets the @p idx 's line from @p str .
         *
         * @throw This function might throw `std::out_of_range` if @p idx
         *        is bigger than the amount of lines in @p str .
         */
        [[nodiscard]]
        auto get_line(const std::string &str, size_t idx) -> std::string;


        [[nodiscard]]
        auto is_word_bound(const unsigned char &ch) -> bool;


        /**
         * Moves @p idx to @p dir direction a word,
         * and returns the current index.
         *
         * @p dir can be -1 for left, and 1 for right
         */
        [[nodiscard]]
        auto move_idx_to_direction(const std::string &str, size_t idx, int dir)
            -> size_t;
    }


    namespace ansi
    {
        [[nodiscard]]
        auto is_arrow(const std::string &seq) -> bool;


        [[nodiscard]]
        auto is_ctrl_pressed(const std::string &seq) -> bool;


        /* returns -1 for home, 1 for end, 0 for not */
        [[nodiscard]]
        auto is_home_end(const std::string &seq) -> int;
    }


    /**
     * @brief Wrapper for `std::getenv`
     */
    [[nodiscard]]
    auto getenv(const std::string &env) -> std::string;


    /**
     * @brief Wrapper for the `std::getenv`
     *
     * when the returned value is nullptr, the function will return
     * @p val instead.
     */
    [[nodiscard]]
    auto getenv(const std::string &env, const std::string &val) -> std::string;


    /**
     * @brief Creates a range from @p start , to @p end .
     */
    template <typename Tp>
    [[nodiscard]]
    constexpr auto
    range(Tp start, Tp end)
    {
        return std::views::iota(start) | std::views::take(end);
    }


#define RUN_FUNC_N(n) \
    for (auto _ : utils::range(static_cast<decltype(n)>(0), n))
}


namespace Json
{
    [[nodiscard]]
    auto to_string(const Json::Value &val) -> std::string;


    [[nodiscard]]
    auto from_string(const std::string &val) -> Json::Value;
}
