#pragma once
#include <ranges>
#include <string>


namespace utils
{
    /**
     * @brief Returns the amount of chars the cursor have to jump to
     *        move out of a word.
     *
     * @param direct The direction to move. 'C' for left, 'D' for right
     */
    [[nodiscard]]
    auto get_cursor_word_jump_offset(const std::string &str,
                                     const size_t      &pos,
                                     const char        &direct) -> size_t;


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
     * @brief Creates a range from @p _0 , to @p _1 .
     */
    template <typename Tp>
    [[nodiscard]]
    auto
    range(Tp _0, Tp _1)
    {
        return std::views::iota(_0) | std::views::take(_1);
    }
}
