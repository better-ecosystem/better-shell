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

            for (; start < text.length(); start++)
            {
                char c { text[start] };

                if (c == '{')
                    bracket_nest++;
                else if (c == '}')
                {
                    bracket_nest--;
                    if (bracket_nest == 0) return start;
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

            if (text[i] == '-' || text[i] == '{' || text[i] == '"') return;

            size_t start { i };
            while (i < LEN && std::isspace(text[i]) == 0 && text[i] != '$') ++i;

            if (start < i)
                tokens->add_token(TokenType::ARGUMENT, start,
                                  text.substr(start, i - start));
        }


        [[nodiscard]]
        auto
        handle_substitution(const shared_tokens &tokens,
                            size_t              &i,
                            const std::string   &text) -> bool
        {
            if (text[i] != '{') return false;

            const size_t start_pos { i };
            i++; /* skip { */

            tokens->add_token(TokenType::SUB_BRACKET, start_pos, "{");

            size_t end_idx { find_last_close_bracket(text, i) };
            if (end_idx == std::string::npos)
            {
                std::string inner { utils::str::trim(text.substr(i)) };
                if (!inner.empty())
                {
                    shared_tokens inner_tokens { parser::parse(inner, tokens) };

                    tokens->add_token(TokenType::SUB_CONTENT, i, inner_tokens);
                }

                i = text.length();
                return true;
            }

            std::string inner { utils::str::trim(text.substr(i, end_idx - i)) };
            if (!inner.empty())
            {
                shared_tokens inner_tokens { parser::parse(inner, tokens) };

                tokens->add_token(TokenType::SUB_CONTENT, i, inner_tokens);
            }


            tokens->add_token(TokenType::SUB_BRACKET, end_idx, "}");

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
                tokens->add_token(TokenType::FLAG, i, text.substr(i, len));
                i += len - 1;
                return true;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len { 1 };
            while (i + len < LEN && std::isspace(text[i + len]) == 0
                   && text[i + len] != '=')
            {
                tokens->add_token(TokenType::FLAG, i, "-"s + text[i + len]);
                len++;
            }

            i += len;
            return true;
        }


        [[nodiscard]]
        auto
        handle_string(const shared_tokens &tokens,
                      size_t              &i,
                      const std::string   &text) -> bool
        {
            if (text[i] != '"') return false;

            tokens->add_token(TokenType::STRING_QUOTE, i, "\"");
            i++;

            size_t      quote_pos { text.find('"', i) };
            std::string content;

            if (quote_pos == std::string::npos)
            {
                content = text.substr(i);
                tokens->add_token(TokenType::STRING_CONTENT, i, content);
                i += content.length();
                return true;
            }

            content = text.substr(i, quote_pos - i);
            if (!content.empty())
            {
                tokens->add_token(TokenType::STRING_CONTENT, i, content);
                i += content.length();
            }

            tokens->add_token(TokenType::STRING_QUOTE, i, "\"");
            i++;

            return true;
        }
    }


    auto
    parse(const std::string &text, const shared_tokens &parent) -> shared_tokens
    {
        auto tokens { std::make_shared<TokenGroup>(text, parent) };

        if (std::string cmd { get_command(text) }; !cmd.empty())
            tokens->add_token(TokenType::COMMAND, 0, cmd);
        size_t i { tokens->tokens.back().get_data<std::string>()->length() };

        handle_argument(tokens, i, text);

        for (; i < text.length(); i++)
        {
            if (std::isspace(text[i]) != 0) continue;

            if (handle_string(tokens, i, text)) continue;
            if (handle_substitution(tokens, i, text)) continue;
            if (handle_flag(tokens, i, text)) continue;
        }

        return tokens;
    }
}
