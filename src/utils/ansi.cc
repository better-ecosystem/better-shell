#include <algorithm>

#include "utils.hh"


namespace utils::ansi
{
    auto
    is_arrow(const std::string &seq) -> bool
    {
        const char BACK { seq.back() };
        return BACK >= 'A' && BACK <= 'D';
    }


    auto
    is_ctrl_pressed(const std::string &seq) -> bool
    {
        size_t idx { seq.find_first_of(';') };
        if (idx == std::string::npos) return false;
        return seq.at(idx + 1) == '5' || seq.at(idx + 1) == '6';
    }


    auto
    is_shift_pressed(const std::string &seq) -> bool
    {
        size_t idx { seq.find_first_of(';') };
        if (idx == std::string::npos) return false;
        return seq.at(idx + 1) == '2' || seq.at(idx + 1) == '6';
    }


    auto
    is_home_end(const std::string &seq) -> int
    {
        const char BACK { seq.back() };

        if (BACK == 'H') return -1;
        if (BACK == 'F') return 1;
        if (BACK == '~')
        {
            if (seq[1] == '1') return -1;
            if (seq[1] == '4') return 1;
        }

        return 0;
    }


    auto
    get_ansi_color(const std::string &seq) -> std::array<int, 3>
    {
        auto parts { utils::str::split(seq.substr(1, seq.length() - 2), ';') };

        /* 255 or RGB */
        if (parts.size() >= 3)
        {
            /* 255 */
            if (parts[1] == "5")
            {
                if (parts.size() != 3) return INVALID_COLOR;

                int value { std::stoi(parts[2]) };
                if (value < 8) return COLORS[value];
                if (value < 16) return COLORS[value - 8];
                if (value > 231)
                {
                    int gray { 8 + ((value - 232) * 10) };
                    return { gray, gray, gray };
                }

                /* 6 × 6 × 6 cube */
                auto comp { [](int v) -> int
                            { return v == 0 ? 0 : 55 + (v * 40); } };

                int index { value - 16 };
                int r { index / 36 };
                int g { (index / 6) % 6 };
                int b { index % 6 };

                return { comp(r), comp(g), comp(b) };
            }

            /* RGB */
            if (parts[1] == "2")
            {
                return { std::stoi(parts[2]), std::stoi(parts[3]),
                         std::stoi(parts[4]) };
            }

            return INVALID_COLOR;
        }

        /* 30 - 37 */
        int code { std::clamp(std::stoi(parts.back()) - 30, 0, 7) };
        return COLORS[code];
    }


    auto
    get_highlighted_foreground(const std::string &ansi_seq) -> bool
    {
        std::array<int, 3> rgb { get_ansi_color(ansi_seq) };

        double brightness { (rgb[0] * 299 + rgb[1] * 587 + rgb[2] * 114)
                            / 1000.0F };

        return brightness <= 128;
    }
}
