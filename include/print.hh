#pragma once
#ifndef __BetterShell__print_hh
#define __BetterShell__print_hh
#include <ostream>
#include <format>

namespace std { extern ostream cout; }


namespace io
{
    template<typename ...T_Args>
    inline void
    print( std::ostream                 &p_stream,
           std::format_string<T_Args...> p_fmt,
           T_Args                   &&...p_args )
    {
        auto fmt_args { std::make_format_args(p_args...) };
        std::string buff { std::vformat(p_fmt.get(), fmt_args) };

        p_stream.write(buff.data(), static_cast<std::streamsize>(buff.size()));

        if (p_stream.bad())
            throw std::system_error(errno, std::generic_category(),
                                   "std::ostream::write failed");
    }


    template<typename ...T_Args>
    inline void
    print( std::format_string<T_Args...> p_fmt, T_Args &&...p_args )
    { print(std::cout, p_fmt, std::forward<T_Args>(p_args)...); }


    template<typename ...T_Args>
    inline void
    println( std::ostream                 &p_stream,
             std::format_string<T_Args...> p_fmt,
             T_Args                   &&...p_args )
    {
        print(p_stream, "{}\n",
              std::format(p_fmt, std::forward<T_Args>(p_args)...));
    }


    template<typename ...T_Args>
    inline void
    println( std::format_string<T_Args...> p_fmt, T_Args &&...p_args )
    { println(std::cout, p_fmt, std::forward<T_Args>(p_args)...); }
}

#endif /* __BetterShell__print_hh */