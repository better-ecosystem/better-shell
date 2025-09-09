#pragma once
#include <string>


namespace utils
{
    /**
     * @brief Returns the amount of chars the cursor have to jump to
     *        move out of a word.
     *
     * @param p_direct The direction to move. 'C' for left, 'D' for right
     */
    [[nodiscard]]
    auto get_cursor_word_jump_offset( const std::string &p_str,
                                      const size_t      &p_pos,
                                      const char        &p_direct ) -> size_t;



    /**
     * @brief Gets the @p p_idx 's line from @p p_str .
     *
     * @throw This function might throw `std::out_of_range` if @p p_idx
     *        is bigger than the amount of lines in @p p_str .
     */
    [[nodiscard]]
    auto get_line( const std::string &p_str, size_t p_idx ) -> std::string;
}