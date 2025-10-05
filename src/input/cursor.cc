#include <algorithm>

#include "input/cursor.hh"
#include "print.hh"
#include "utils.hh"

using input::term::Cursor;


namespace
{
    [[nodiscard]]
    auto
    is_word_bound(gunichar c) -> bool
    {
        return ((g_unichar_isspace(c) != 0) || (g_unichar_ispunct(c) != 0));
    }
}


auto
Cursor::is_zero() const noexcept -> bool
{
    return x == 0 && y == 0;
}


auto
Cursor::handle_arrows(Direction dir, const Glib::ustring &str, bool ctrl)
    -> bool
{
    switch (dir)
    {
    case DIR_UP:    return handle_up_arrow();
    case DIR_DOWN:  return handle_down_arrow(str);
    case DIR_RIGHT: return handle_right_arrow(str, ctrl);
    case DIR_LEFT:  return handle_left_arrow(str, ctrl);
    }
}


auto
Cursor::get_string_idx(const Glib::ustring &str) const -> size_t
{
    size_t line { 0 };
    size_t char_x { 0 };

    for (size_t i = 0; i < str.size();)
    {
        if (line == y && char_x == x) return i;

        gunichar c { str[i] };

        if (c == '\n')
        {
            if (line == y && x == 0) return i;
            line++;
            char_x = 0;
            i     += 1;
            continue;
        }

        if (line == y) char_x++;

        i += 1;
    }

    if (line == y && char_x == x) return str.size();

    throw std::out_of_range("Coordinates out of range");
}


auto
Cursor::handle_up_arrow() -> bool
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
Cursor::handle_down_arrow(const Glib::ustring &str) -> bool
{
    /*
     * DOWN: If not on the last line, cursor should move down 1 line,
     *       else, move to the next ran command, or do nothing.
    */
    if (y <= std::ranges::count(str, '\n'))
    {
        io::print("\033[B");
        y++;
        return true;
    }

    return false;
}


auto
Cursor::handle_right_arrow(const Glib::ustring &str, bool ctrl) -> bool
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
    if (ctrl)
    {
        uint32_t      start_x { x };
        Glib::ustring line { utils::get_line(str, y) };
        const size_t  len { line.length() };

        if (x <= len && is_word_bound(line[x])) x++;
        while (x < len && !is_word_bound(line[x])) x++;
        max_x = x;

        for (uint32_t _ { 0 }; _ < (x - start_x); _++) io::print("\033[C");
    }
    else if (x < utils::get_line(str, y).length())
    {
        io::print("\033[C");
        x++;
        return true;
    }

    size_t line_amount { static_cast<size_t>(std::ranges::count(str, '\n')) };
    if (y < line_amount)
    {
        io::print("\033[G");
        io::print("\033[B");

        x = 0;
        y++;
    }

    return true;
}


auto
Cursor::handle_left_arrow(const Glib::ustring &str, bool ctrl) -> bool
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
    if (ctrl)
    {
        uint32_t      start_x { x };
        Glib::ustring line { utils::get_line(str, y) };
        if (x > 0 && is_word_bound(line[x - 1])) x--;
        while (x > 0 && !is_word_bound(line[x - 1])) x--;
        max_x = x;

        for (auto _ : utils::range(0U, start_x - x)) io::print("\033[D");

        return true;
    }

    if (x > 0)
    {
        io::print("\033[D");
        x--;

        return true;
    }

    if (y > 0)
    {
        io::print("\033[A");
        y--;

        x = utils::get_line(str, y).length() - 1;
        io::print("\033[{}G", x + 1);
    }

    return true;
}
