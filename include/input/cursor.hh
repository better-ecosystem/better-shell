#pragma once
#include <cstdint>

#include <glibmm/ustring.h>


namespace input::term
{
    struct Cursor
    {
        enum Direction : uint8_t
        {
            DIR_UP    = 'A',
            DIR_DOWN  = 'B',
            DIR_RIGHT = 'C',
            DIR_LEFT  = 'D',
        };


        uint32_t x { 0 };
        uint32_t y { 0 };

        uint32_t last_x { 0 };
        uint32_t max_x { 0 };


        [[nodiscard]]
        auto is_zero() const noexcept -> bool;


        /**
         * @brief Handles arrow key input from the shell.
         *
         * @return true if something has changed,
         *         or false if nothing has changed.
         */
        [[nodiscard]]
        auto handle_arrows(Direction dir, const Glib::ustring &str, bool ctrl)
            -> bool;


        /**
         * @brief Gets the absolute index of @p str
         *        from the x, and y position.
         *
         * @throw The function might throw an `std::out_of_range`
         *        if the function can't get to the desired index without
         *        going out of range from the string's length.
         */
        [[nodiscard]]
        auto get_string_idx(const Glib::ustring &str) const -> size_t;

    private:
        [[nodiscard]]
        auto handle_up_arrow() -> bool;


        [[nodiscard]]
        auto handle_down_arrow(const Glib::ustring &str) -> bool;


        [[nodiscard]]
        auto handle_right_arrow(const Glib::ustring &str, bool ctrl) -> bool;


        [[nodiscard]]
        auto handle_left_arrow(const Glib::ustring &str, bool ctrl) -> bool;
    };
}
