#pragma once
#include <format>
#include <ostream>

namespace std { extern ostream cout; }


namespace io
{
    template <typename... T_Args>
    inline void
    print(std::ostream                 &stream,
          std::format_string<T_Args...> fmt,
          T_Args &&...args)
    {
        auto        fmt_args { std::make_format_args(args...) };
        std::string buff { std::vformat(fmt.get(), fmt_args) };

        stream.write(buff.data(), static_cast<std::streamsize>(buff.size()));

        if (stream.bad())
            throw std::system_error(errno, std::generic_category(),
                                    "std::ostream::write failed");
    }


    template <typename... T_Args>
    inline void
    print(std::format_string<T_Args...> fmt, T_Args &&...args)
    {
        print(std::cout, fmt, std::forward<T_Args>(args)...);
    }


    template <typename... T_Args>
    inline void
    println(std::ostream                 &stream,
            std::format_string<T_Args...> fmt,
            T_Args &&...args)
    {
        print(stream, "{}\n",
              std::format(fmt, std::forward<T_Args>(args)...));
    }


    template <typename... T_Args>
    inline void
    println(std::format_string<T_Args...> fmt, T_Args &&...args)
    {
        println(std::cout, fmt, std::forward<T_Args>(args)...);
    }
}
