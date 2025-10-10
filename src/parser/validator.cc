#include <stack>

#include "command/built_in.hh"
#include "command/runner.hh"
#include "parser/error.hh"
#include "parser/types.hh"

using namespace std::literals;
namespace fs = std::filesystem;


namespace parser
{
    namespace
    {
        [[nodiscard]]
        auto
        collect_paths(const fs::path &dir) -> std::vector<fs::path>
        {
            std::vector<fs::path> paths;

            fs::directory_iterator it { dir };
            for (const auto &entry : it)
                paths.emplace_back(fs::relative(fs::canonical(entry.path())));
            return paths;
        }


        [[nodiscard]]
        auto
        find_closest_path(const std::vector<fs::path> &paths,
                          const fs::path &path) -> std::pair<fs::path, int>
        {
            int      smallest { std::numeric_limits<int>::max() };
            fs::path closest;

            for (const auto &candidate : paths)
            {
                fs::path filename { candidate.filename() };
                int dist { utils::str::levenshtein_distance(filename.string(),
                                                            path.string()) };
                if (dist < smallest)
                {
                    closest  = filename;
                    smallest = dist;
                }

                if (dist == 0) break;
            }

            return { closest, smallest };
        }


        [[nodiscard]]
        auto
        find_nearest_looking_path(const fs::path &base) -> fs::path
        {
            std::vector<fs::path> segments;
            for (const auto &part : base) segments.emplace_back(part.string());
            if (segments.empty()) return {};

            std::filesystem::path current_path {
                base.is_absolute() ? base.root_path()
                                   : std::filesystem::current_path()
            };

            for (size_t i { 0 }; i < segments.size(); i++)
            {
                std::vector<fs::path> children;
                try
                {
                    children = collect_paths(current_path);
                }
                catch (...)
                {
                    return {};
                }

                auto [closest,
                      distance] { find_closest_path(children, segments[i]) };

                int min_distance { 2 + static_cast<int>(segments.size() * 2) };
                if (distance > min_distance) return {};

                current_path /= closest;

                if (i < segments.size() - 1 && !fs::is_directory(current_path))
                    return {};
            }

            return fs::relative(current_path);
        }


        [[nodiscard]]
        auto
        handle_path_verification(TokenGroup &tokens, Token &front)
            -> std::pair<bool, std::optional<::error::Info>>
        {
            const std::string *TEXT { front.get_data<std::string>() };

            if (!TEXT->starts_with("./")) return { false, std::nullopt };
            fs::path path { TEXT->substr(2) };

            if (fs::exists(path))
            {
                path = fs::canonical(path);

                if (!fs::is_regular_file(path))
                    return { true, error::create<error::Type::INVALID_COMMAND>(
                                       tokens, front, "path '{}' is not a file",
                                       *TEXT) };

                if (access(path.c_str(), X_OK) != 0)
                    return { true,
                             error::create<error::Type::INVALID_COMMAND>(
                                 tokens, front,
                                 "path '{}' is not an executable", *TEXT) };

                return { false, std::nullopt };
            }

            auto err { error::create<error::Type::INVALID_COMMAND>(
                tokens, front, "executable path '{}' doesn't exist", *TEXT) };

            fs::path match { find_nearest_looking_path(path) };
            if (match.empty()) return { false, err };

            std::string text { "./"s + match.relative_path().string() };

            auto c { ::error::ask<'y', 'y', 'n'>(
                "path '{}' not found, do you mean {}?", *TEXT, text) };

            if (c != 'y') return { false, err };

            path       = match;
            front.data = text;
            return { true, std::nullopt };
        }


        [[nodiscard]]
        auto
        handle_command_verification(TokenGroup &tokens, Token &front)
            -> std::pair<bool, std::optional<::error::Info>>
        {
            const std::string *TEXT { front.get_data<std::string>() };

            if (!cmd::BINARY_PATH_LIST.contains(*TEXT)
                && !cmd::built_in::COMMANDS.contains(*TEXT))
            {
                auto err { error::create<error::Type::INVALID_COMMAND>(
                    tokens, front, "command '{}' doesn't exist", *TEXT) };

                int         smallest { std::numeric_limits<int>::max() };
                std::string bin_name;
                for (const auto &[name, func] : cmd::built_in::COMMANDS)
                {
                    int dist { utils::str::levenshtein_distance(name, *TEXT) };
                    if (dist < smallest)
                    {
                        smallest = dist;
                        bin_name = name;
                    }
                }

                for (const auto &[name, path] : cmd::BINARY_PATH_LIST)
                {
                    int dist { utils::str::levenshtein_distance(name, *TEXT) };
                    if (dist < smallest)
                    {
                        smallest = dist;
                        bin_name = name;
                    }
                }

                if (smallest > 2) return { true, err };
                char res { ::error::ask<'y', 'y', 'n'>(
                    "command '{}' doesn't exist, do you mean '{}'?", *TEXT,
                    bin_name) };

                if (res != 'y') return { true, err };
                front.data = bin_name;

                return { true, std::nullopt };
            }

            return { false, std::nullopt };
        }


        [[nodiscard]]
        auto
        verify_command(TokenGroup &tokens, Token &front)
            -> std::optional<::error::Info>
        {
            auto err { handle_path_verification(tokens, front) };
            if (err.first) return err.second;

            return handle_command_verification(tokens, front).second;
        }


        [[nodiscard]]
        auto
        check_string_quote_token(TokenGroup &tokens,
                                 size_t     &quote_idx,
                                 size_t idx) -> std::optional<::error::Info>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::STRING_QUOTE)
            {
                if (quote_idx != std::string::npos)
                {
                    if (quote_idx + 1 < tokens.tokens.size()
                        && tokens.tokens[quote_idx + 1].type
                               != TokenType::STRING_CONTENT)
                    {
                        return error::create<error::Type::EMPTY_STRING>(
                            tokens, tokens.tokens[quote_idx],
                            "string is empty");
                    }
                    quote_idx = std::string::npos;
                }
                else
                    quote_idx = idx;
            }
            return std::nullopt;
        }


        [[nodiscard]]
        auto
        check_substitution_bracket_token(TokenGroup         &tokens,
                                         std::stack<size_t> &bracket_stack,
                                         size_t              idx)
            -> std::optional<::error::Info>
        {
            const auto &token { tokens.tokens[idx] };

            if (token.type == TokenType::SUB_BRACKET)
            {
                const std::string *text { token.get_data<std::string>() };
                const char         bracket { (*text)[0] };

                if (bracket == '{')
                {
                    bracket_stack.push(idx);

                    if (idx + 1 < tokens.tokens.size()
                        && tokens.tokens[idx + 1].type
                               != TokenType::SUB_CONTENT)
                    {
                        return error::create<error::Type::EMPTY_SUBSTITUTION>(
                            tokens, tokens.tokens[idx],
                            "substitution has no content");
                    }
                }
                else if (bracket == '}')
                {
                    if (!bracket_stack.empty())
                        bracket_stack.pop();
                    else
                    {
                        /* unexpected closing bracket */
                        return error::create<error::Type::UNCLOSED_BRACKET>(
                            tokens, tokens.tokens[idx],
                            "unmatched closing bracket");
                    }
                }
            }

            return std::nullopt;
        }


        [[nodiscard]]
        auto
        check_parameter_token(TokenGroup &tokens, size_t idx)
            -> std::optional<::error::Info>
        {
            if (tokens.tokens[idx].type != TokenType::PARAMETER)
                return std::nullopt;

            const auto *text { tokens.tokens[idx].get_data<std::string>() };

            if (!text->empty()) return std::nullopt;

            return error::create<error::Type::EMPTY_PARAM>(
                tokens, tokens.tokens[idx], "parameter not given");
        }
    }


    auto
    TokenGroup::verify_syntax() -> std::optional<::error::Info>
    {
        using enum TokenType;

        auto err { verify_command(*this, this->tokens.front()) };
        if (err) return err;

        std::stack<size_t> bracket_stack;
        size_t             quote_idx { std::string::npos };

        for (size_t i { 1 }; i < this->tokens.size(); i++)
        {
            if (this->tokens[i].type == SUB_CONTENT)
            {
                auto res { this->tokens[i]
                               .get_data<shared_tokens>()
                               ->get()
                               ->verify_syntax() };
                if (res) return res;
            }

            auto res { check_parameter_token(*this, i) };
            if (res) return res;

            res = check_string_quote_token(*this, quote_idx, i);
            if (res) return res;

            res = check_substitution_bracket_token(*this, bracket_stack, i);
            if (res) return res;
        }

        if (!bracket_stack.empty())
            return error::create<error::Type::UNCLOSED_BRACKET>(
                *this, tokens[bracket_stack.top()], "unclosed bracket");

        if (quote_idx != std::string::npos)
            return error::create<error::Type::UNCLOSED_QUOTE>(
                *this, tokens[quote_idx], "unclosed quote");


        return std::nullopt;
    }
}
