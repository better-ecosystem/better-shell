#pragma once
#include <format>
#include <iostream>
#include <utility>

#include "print.hh"
#include "utils.hh"

#ifndef NDEBUG
#include <source_location>
#endif


namespace error
{
    namespace color
    {
        inline std::string_view ERROR { ANSI_RGB_FG(253, 106, 106) };
        inline std::string_view MESSAGE { ANSI_RGB_FG(70, 172, 173) };
        inline std::string_view LINE_NUM { ANSI_RGB_FG(150, 150, 150) };
        constexpr static auto   RESET { COLOR_RESET };

        inline std::string_view LINE_BG { ANSI_RGB_BG(30, 30, 30) };
        inline std::string_view LINE_BG_ALT { ANSI_RGB_BG(45, 45, 45) };
    }


    /**
     * prompt a question to the user
     * -----------------------------
     *
     * the function will read for a single character input
     * and checks if it is present in @p T_Options
     * if the inputted character is a newline character
     * the function will return @p T_Default
     *
     * set @p T_Default to '\0', to disable default option
     */
    template <char T_Default, char... T_Options, typename... T_Args>
    constexpr auto
    ask(std::string_view fmt, T_Args &&...args) -> char
    {
        std::string msg { std::vformat(fmt, std::make_format_args(args...)) };

        io::print("{}ask:{} {} [", color::MESSAGE, color::RESET, msg);
        ((io::print("{}", (T_Options == T_Default
                               ? static_cast<char>(std::toupper(T_Options))
                               : T_Options))),
         ...);
        io::print("] ");

        constexpr std::array<char, sizeof...(T_Options)> VALID_OPTIONS {
            T_Options...
        };

        int ch { std::getchar() };
        std::cerr.put(ch);
        std::cerr.put('\n');

        if (T_Default != '\0')
        {
            if (ch == EOF || ch == 4) return T_Default;
            if (ch == '\n' || ch == '\r') return T_Default;
        }

        for (char opt : VALID_OPTIONS)
            if (std::tolower(ch) == std::tolower(opt)) return opt;

        io::println("{}error:{} invalid input, please enter the given options",
                    color::ERROR, color::RESET);
        return ask<T_Default, T_Options...>(msg);
    }


    /**
     * holds the information for errors, and the pretty message
     */
    class Info
    {
    public:
        template <typename... T_Args>
        Info(std::string error_type, std::string_view fmt, T_Args &&...args)
            : m_error_type(std::move(error_type)),
              m_msg(std::vformat(fmt, std::make_format_args(args...)))
        {
        }


        /**
         * specify the error context
         * -------------------------
         *
         * if any of the string parameters were empty,
         * all of the parameter were to be considered invalid
         */
        void set_error_context(const std::string &input_source,
                               const std::string &text,
                               size_t             idx,
                               size_t             len);





        /**
         * creates a formatted pretty message containing the error
         * -------------------------------------------------------
         *
         * @p force forces whether the pretty message should be reformatted
         * instead of just using the buffer
         */
        [[nodiscard]]
        auto create_pretty_message(bool force = false) -> std::string;

    private:
        std::string m_error_type;
        std::string m_pretty_msg;
        std::string m_msg;

        std::vector<std::string> m_error_lines;
        std::string              m_input_source;
        std::spair<size_t>       m_error_pos;
        size_t                   m_error_len;
    };


#ifndef NDEBUG
    constexpr void
    impl_assert(bool                        expr,
                std::string_view            expr_string,
                std::string_view            message,
                const std::source_location &source
                = std::source_location::current())
    {
        if (expr) return;

        io::println(
            std::cerr, "{}error:{}   assertion ({}{}{}) failed [{}:{}:{}]",
            color::ERROR, color::RESET, color::MESSAGE, expr_string,
            color::RESET, source.file_name(), source.line(), source.column());
        io::println(std::cerr, "{}message:{} {}", color::MESSAGE, color::RESET,
                    message);
        std::exit(1);
    }


#define assert(expr, msg) impl_assert(expr, #expr, msg)
#else
#define impl_assert(...)
#define assert(expr)
#endif
}
