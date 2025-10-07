#include "parser/parser.hh"
#include "utils.hh"

using namespace std::literals;


namespace parser
{
    namespace
    {
        /**
         * find the last close bracket
         * ---------------------------
         *
         * returns std::string::npos on "unmatched closing bracket",
         * or returns the length of @p text if no closing bracket is found
         */
        [[nodiscard]]
        auto
        find_last_close_bracket(const std::string &text, size_t start) -> size_t
        {
            size_t bracket_nest { 1 };
            bool   in_single_quote { false };
            bool   in_double_quote { false };

            for (size_t i { start }; i < text.size(); i++)
            {
                char c { text[i] };

                if (c == '\'' && !in_double_quote)
                    in_single_quote = !in_single_quote;
                else if (c == '"' && !in_single_quote)
                    in_double_quote = !in_double_quote;

                if (in_single_quote || in_double_quote) continue;

                if (c == BracketKind::OPEN)
                    bracket_nest++;
                else if (c == BracketKind::CLOSE)
                {
                    bracket_nest--;
                    if (bracket_nest == 0) return i;
                }
            }

            return std::string::npos;
        }


        [[nodiscard]]
        auto
        get_command(const std::string &str) -> std::string
        {
            std::istringstream iss { str };
            std::string        word;
            iss >> word;
            return word;
        }


        void
        handle_argument(const shared_tokens &tokens,
                        size_t              &i,
                        const std::string   &text)
        {
            const size_t LEN { text.length() };

            while (i < LEN && std::isspace(text[i]) != 0) i++;
            if (i >= LEN) return;

            if (text[i] == '-') return;

            if (text[i] == '"' || text[i] == '\'')
            {
                char   quote { text[i++] };
                size_t start { i };

                while (i < LEN && text[i] != quote) ++i;

                if (i >= LEN)
                    throw std::invalid_argument("Unterminated quoted argument");

                tokens->tokens.emplace_back(TokenType::ARGUMENT, start,
                                            text.substr(start, i - start));
                i++;
                return;
            }

            size_t start { i };
            while (i < LEN && std::isspace(text[i]) == 0 && text[i] != '$') ++i;

            if (start < i)
                tokens->tokens.emplace_back(TokenType::ARGUMENT, start,
                                            text.substr(start, i - start));
        }


        [[nodiscard]]
        auto
        handle_substitution(const shared_tokens &tokens,
                            size_t              &i,
                            const std::string   &text) -> bool
        {
            if (text[i] != '$' || i + 1 >= text.size() || text[i + 1] != '{')
                return false;

            const size_t start_pos { i };
            i += 2; /* skip ${ */

            tokens->tokens.emplace_back(TokenType::SUB_BRACKET, start_pos, "${",
                                        BracketKind::OPEN);

            size_t end_idx { find_last_close_bracket(text, i) };
            if (end_idx == std::string::npos)
            {
                std::string inner { utils::str::trim(text.substr(i)) };
                if (!inner.empty())
                {
                    shared_tokens inner_tokens { parser::parse(inner, tokens) };

                    tokens->tokens.emplace_back(TokenType::SUB_CONTENT, i,
                                                inner_tokens);
                }

                i = text.length();
                return true;
            }

            std::string inner { utils::str::trim(text.substr(i, end_idx - i)) };
            if (!inner.empty())
            {
                shared_tokens inner_tokens { parser::parse(inner, tokens) };

                tokens->tokens.emplace_back(TokenType::SUB_CONTENT, i,
                                            inner_tokens);
            }


            tokens->tokens.emplace_back(TokenType::SUB_BRACKET, end_idx, "}",
                                        BracketKind::CLOSE);

            i = end_idx;
            return true;
        }


        [[nodiscard]]
        auto
        handle_flag(const shared_tokens &tokens,
                    size_t              &i,
                    const std::string   &text) -> bool
        {
            if (i > 0 && (text[i] != '-' || std::isspace(text[i - 1]) == 0))
                return false;

            const size_t LEN { text.length() };

            /* Handles long arg */
            if (i + 1 < LEN && text[i + 1] == '-')
            {
                size_t len { 2 };
                while (i + len < LEN && std::isspace(text[i + len]) == 0) len++;
                tokens->tokens.emplace_back(
                    TokenType::FLAG, i, text.substr(i, len), FlagKind::LONG);
                i += len - 1;
                return true;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len { 1 };
            while (i + len < LEN && std::isspace(text[i + len]) == 0
                   && text[i + len] != '=')
            {
                tokens->tokens.emplace_back(
                    TokenType::FLAG, i, "-"s + text[i + len], FlagKind::SHORT);
                len++;
            }

            i += len;
            return true;
        }
    }


    auto
    parse(const std::string &text, const shared_tokens &parent) -> shared_tokens
    {
        auto tokens { std::make_shared<TokenGroup>(text, parent) };

        if (std::string cmd { get_command(text) }; !cmd.empty())
            tokens->tokens.emplace_back(TokenType::COMMAND, 0, cmd);
        size_t i { tokens->tokens.back().get_data<std::string>()->length() };

        handle_argument(tokens, i, text);

        for (; i < text.length(); i++)
        {
            if (std::isspace(text[i]) != 0) continue;

            if (handle_substitution(tokens, i, text)) continue;
            if (handle_flag(tokens, i, text)) continue;
        }

        return tokens;
    }
}
