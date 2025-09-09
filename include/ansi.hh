#pragma once
#include <string_view>


/**
 * @brief This namespace contains constexpr ANSI escape codes.
 */
namespace ansi
{
#define ANSI_String static constexpr std::string_view
#define ANSI_Char   static constexpr char

    ANSI_String ESCAPE1 { "\033[" };
    ANSI_String ESCAPE2 { "\x1b[" };

    ANSI_Char CURSOR_UP    { 'A' };
    ANSI_Char CURSOR_DOWN  { 'B' };
    ANSI_Char CURSOR_RIGHT { 'C' };
    ANSI_Char CURSOR_LEFT  { 'D' };
    ANSI_Char CURSOR_NEXT_LN { 'E' };
    ANSI_Char CURSOR_PREV_LN { 'E' };


    /**
     * @brief Checks whether a string @p p_str starts with an ANSI escape code.
     */
    [[nodiscard]]
    auto is_escape( std::string_view p_str ) -> bool;
}