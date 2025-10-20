#pragma once
#include <charconv>
#include <optional>
#include <string>
#include <vector>


struct ArgIndex
{
    int                arg_idx;
    std::optional<int> char_idx;


    constexpr ArgIndex(size_t i, size_t j) : arg_idx(i), char_idx(j) {}
    constexpr ArgIndex(size_t i) : arg_idx(i), char_idx(std::nullopt) {}
};


class ArgParser
{
public:
    ArgParser(int &argc, char **&argv);


    /**
     * returns the index of the flag if it exist
     * ---------------------------------------------
     *
     * if @p short_flag exist the function will return the
     * index of the short flag in @e argv and inside @e argv[index]
     *
     * and if @p long_flag exist, the function will only return the index
     * of the flag inside @e argv
     */
    [[nodiscard]]
    auto is_flag(std::string_view long_flag, char short_flag, bool accept_param)
        -> std::optional<ArgIndex>;


    /**
     * returns the index of the argument if it exist
     * ---------------------------------------------
     *
     * if the arg exist the function will return the
     * index of the arg in @e argv
     */
    [[nodiscard]]
    auto is_arg(std::string_view long_arg, char short_arg, bool accept_param)
        -> std::optional<ArgIndex>;


    /**
     * returns the option of the argument in @p index
     * ----------------------------------------------
     *
     * @p index is the index of the argument given by
     * the public method @e is_arg
     */
    template <typename Tp>
    [[nodiscard]]
    auto
    get_parameter(const ArgIndex &index) -> std::optional<Tp>
    {
        if (!index.char_idx)
        {
            const auto idx { static_cast<size_t>(index.arg_idx) };

            if (size_t pos { m_args[idx].find('=') }; pos != std::string::npos)
            {
                std::string param { m_args[idx].substr(pos + 1) };
                m_args[idx].erase(pos);
                return to_type<Tp>(param);
            }

            if (m_args.size() > idx + 1 && !m_args[idx + 1].starts_with('-'))
            {
                std::string param { m_args[idx + 1] };

                m_args.erase(m_args.begin() + idx + 1);
                m_args.erase(m_args.begin() + idx);

                return to_type<Tp>(param);
            }

            return std::nullopt;
        }

        const auto first { static_cast<size_t>(index.arg_idx) };
        const auto second { static_cast<size_t>(*index.char_idx) };

        std::string &arg { m_args[first] };

        if (second + 1 < arg.length() && arg[second + 1] == '=')
        {
            std::string param { arg.substr(second + 2) };
            arg.erase(second);
            return to_type<Tp>(param);
        }
        if (arg.length() - 1 == second && m_args.size() > first + 1
            && !m_args[first + 1].starts_with('-'))
        {
            std::string param { m_args[first + 1] };
            arg.pop_back();

            m_args.erase(m_args.begin() + first + 1);
            return to_type<Tp>(param);
        }

        std::string param { arg.substr(second + 1) };
        arg.erase(second);
        return to_type<Tp>(param);
    }


    [[nodiscard]]
    auto get_args() const -> std::vector<std::string>;

private:
    std::vector<std::string> m_args;


    template <typename Tp>
    [[nodiscard]]
    static auto
    to_type(const std::string &str) -> std::optional<Tp>
    {
        if constexpr (std::is_same_v<Tp, std::string>) return str;

        if constexpr (std::is_integral_v<Tp>)
        {
            Tp val;
            if (std::from_chars(str.begin(), str.end(), val).ec == std::errc {})
                return val;
            return std::nullopt;
        }

        if constexpr (std::is_same_v<Tp, bool>)
        {
            if (str == "true") return true;
            if (str == "false") return false;
        }

        return std::nullopt;
    }
};
