#include <algorithm>
#include "input/cursor_pos.hh"
#include "print.hh"
#include "utils.hh"

using input::term::CursorPosition;


namespace
{
    [[nodiscard]]
    auto
    is_word_bound( char p_c ) -> bool
    { return ((std::isspace(p_c) != 0) || (std::ispunct(p_c) != 0)); }
}


auto
CursorPosition::is_zero() const noexcept -> bool
{ return x == 0 && y == 0; }


auto
CursorPosition::handle_arrows( Direction          p_dir,
                               const std::string &p_str,
                               bool               p_ctrl ) -> bool
{
    switch (p_dir) {
    case DIR_UP:    return handle_up_arrow();
    case DIR_DOWN:  return handle_down_arrow (p_str);
    case DIR_RIGHT: return handle_right_arrow(p_str, p_ctrl);
    case DIR_LEFT:  return handle_left_arrow (p_str, p_ctrl);
    }
}


auto
CursorPosition::get_string_idx( const std::string &p_str ) const -> size_t
{
    size_t line   { 0 };
    size_t char_x { 0 };

    for (size_t i = 0; i < p_str.size();) {
        if (line == y && char_x == x) return i;

        unsigned char c { static_cast<unsigned char>(p_str[i]) };

        size_t advance =
            (c < 0x80) ? 1 :
            (c < 0xE0) ? 2 :
            (c < 0xF0) ? 3 : 4;

        if (c == '\n') {
            if (line == y && x == 0) return i;
            line++;
            char_x = 0;
            i += 1;
            continue;
        }

        if (line == y) char_x++;

        i += advance;
    }

    if (line == y && char_x == x) return p_str.size();

    throw std::out_of_range("Coordinates out of range");
}


auto
CursorPosition::handle_up_arrow() -> bool
{
    /*
     * UP: If not on the first line, cursor should move up 1 line,
     *     else, move to the previously ran command.
    */
    if (y == 0) return false;

    io::print("\033[A");
    y--;
    return true;
}


auto
CursorPosition::handle_down_arrow( const std::string &p_str ) -> bool
{
    /*
     * DOWN: If not on the last line, cursor should move down 1 line,
     *       else, move to the next ran command, or do nothing.
    */
    if (y <= std::ranges::count(p_str, '\n')) {
        io::print("\033[B");
        y++;
        return true;
    }

    return false;
}


auto
CursorPosition::handle_right_arrow( const std::string &p_str,
                                    bool               p_ctrl ) -> bool
{
    /*
     * CTRL + RIGHT: If not on the last character of the line,
     *               move cursor right until punctuation/space.
     *               Else, move to the next line, and put the
     *               cursor at the start of the line, or do nothing.
     *
     * RIGHT: If not on the last character of the line, move cursor right once,
     *        else, move to the next line, and put cursor at
     *        the start of the line, or do nothing.
    */
    if (p_ctrl) {
        uint32_t start_x { x };
        std::string line { utils::get_line(p_str, y) };
        const size_t len { line.length() };

        if (x <= len && is_word_bound(line[x])) x++;
        while (x < len && !is_word_bound(line[x])) x++;
        max_x = x;

        for (uint32_t _ { 0 }; _ < (x - start_x); _++)
            io::print("\033[C");
    } else if (x < utils::get_line(p_str, y).length()) {
        io::print("\033[C");
        x++;
        return true;
    }

    size_t line_amount { static_cast<size_t>(std::ranges::count(p_str, '\n')) };
    if (y < line_amount) {
        io::print("\033[G");
        io::print("\033[B");

        x = 0;
        y++;
    }

    return true;
}


auto
CursorPosition::handle_left_arrow( const std::string &p_str,
                                   bool               p_ctrl ) -> bool
{
    /*
     * CTRL + LEFT: If not on the first character of the line,
     *              move cursor left until punctuation/space.
     *              Else, move to the previous line, and put
     *              cursor at the end of the line, or do nothing.
     *
     * LEFT: If not on the first character of the line, move cursor left once,
     *       else, move to the previous line, and put cursor at
     *       the last character of the line, or do nothing.
    */
    if (p_ctrl) {
        uint32_t start_x { x };
        std::string line { utils::get_line(p_str, y) };
        if (x > 0 && is_word_bound(line[x - 1])) x--;
        while (x > 0 && !is_word_bound(line[x - 1])) x--;
        max_x = x;

        for (uint32_t _ { 0 }; _ < (start_x - x); _++)
            io::print("\033[D");

        return true;
    }

    if (x > 0) {
        io::print("\033[D");
        x--;

        return true;
    }

    if (y > 0) {
        io::print("\033[A");
        y--;

        x = utils::get_line(p_str, y).length() - 1;
        io::print("\033[{}G", x + 1);
    }

    return true;
}