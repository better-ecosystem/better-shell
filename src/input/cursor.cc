#include <algorithm>

#include "input/cursor.hh"
#include "print.hh"
#include "utils.hh"

using input::term::Cursor;


auto
Cursor::is_zero() const noexcept -> bool
{
    return x == 0 && y == 0;
}


auto
Cursor::handle_arrows(Direction dir, const std::string &str, bool ctrl) -> bool
{
    switch (dir)
    {
    case DIR_RIGHT: return handle_right_arrow(str, ctrl);
    case DIR_LEFT:  return handle_left_arrow(str, ctrl);
    default:        return false;
    }
}


auto
Cursor::handle_home_end(int type, const std::string &str, bool ctrl) -> bool
{
    if (!ctrl)
    {
        if (type == -1) /* home */
        {
            while (x > 0)
            {
                io::print("\033[D");
                x--;
            }
        }
        else if (type == 1) /* end */
        {
            std::string  line { utils::str::get_line(str, y) };
            const size_t LEN { line.length() };

            while (x < LEN)
            {
                io::print("\033[C");
                x++;
            }
        }

        return true;
    }

    return false;
}


auto
Cursor::get_string_idx(const std::string &str) const -> size_t
{
    size_t line { 0 };
    size_t char_x { 0 };

    for (size_t i = 0; i < str.size();)
    {
        if (line == y && char_x == x) return i;

        unsigned char c { static_cast<unsigned char>(str[i]) };
        const size_t  advance { utils::utf8::get_expected_length(c) };

        if (c == '\n')
        {
            if (line == y && x == 0) return i;
            line++;
            char_x = 0;
            i     += 1;
            continue;
        }

        if (line == y) char_x++;

        i += advance;
    }

    if (line == y && char_x == x) return str.size();

    throw std::out_of_range(
        std::format("Coordinates out of range (x: {} y: {})", x, y));
}


auto
Cursor::handle_right_arrow(const std::string &str, bool ctrl) -> bool
{
    std::string  line { utils::str::get_line(str, y) };
    const size_t LEN { line.length() };

    if (ctrl)
    {
        uint32_t start_x { x };
        x = utils::str::move_idx_to_direction(line, x, 1);

        max_x = x;
        RUN_FUNC_N(x - start_x, io::print, "\033[C");
    }
    else
    {
        if (x < LEN)
        {
            io::print("\033[C");
            x++;
        }
        else if (y < std::ranges::count(str, '\n'))
        {
            io::print("\033[G");
            io::print("\033[B");

            x = 0;
            y++;
        }
    }

    return true;
}


auto
Cursor::handle_left_arrow(const std::string &str, bool ctrl) -> bool
{
    std::string line { utils::str::get_line(str, y) };

    if (ctrl)
    {
        uint32_t start_x { x };
        x = utils::str::move_idx_to_direction(line, x, -1);

        max_x = x;
        RUN_FUNC_N(start_x - x, io::print, "\033[D");
    }
    else
    {
        if (x > 0)
        {
            io::print("\033[D");
            x--;
        }
        else if (y > 0)
        {
            y--;
            line = utils::str::get_line(str, y);
            x    = line.length() - 1;

            io::print("\033[A");
            RUN_FUNC_N(x + 1, io::print, "\033[G");
        }
    }

    return true;
}


auto
Cursor::handle_up_arrow(const std::string &str) -> bool
{
    if (y == 0) return false;
    std::spair old { x, y };

    while (x > 0)
    {
        io::print("\033[D");
        x--;
    }

    return true;
}


auto
Cursor::handle_down_arrow(const std::string &str) -> bool
{
    if (y <= std::ranges::count(str, '\n'))
    {
        io::print("\033[B");
        y++;
        return true;
    }

    return false;
}