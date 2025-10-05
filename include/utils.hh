#pragma once
#include <ranges>
#include <string>


namespace utils
{
    /**
     * @brief Gets the @p idx 's line from @p str .
     *
     * @throw This function might throw `std::out_of_range` if @p idx
     *        is bigger than the amount of lines in @p str .
     */
    [[nodiscard]]
    auto get_line(const std::string &str, size_t idx) -> std::string;


    /**
     * @brief Checks whether @p str contains a unicode utf8 value.
     */
    [[nodiscard]]
    auto contains_unicode(std::string_view str) -> bool;


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
    auto
    range(Tp start, Tp end)
    {
        return std::views::iota(start) | std::views::take(end);
    }
}
