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


        /**
         * determines if a char @p ch belongs to token attribute
         * -----------------------------------------------------
         *
         * for example, the char '{' belongs to TokenType::SUB_QUOTE
         * so the function will return true
         */
        [[nodiscard]]
        auto
        char_belongs_to_token(char ch) -> bool
        {
            return ch == '-' || ch == '{' || ch == '"' || ch == '!' || ch == '|'
                || ch == '&' || ch == ';' || ch == ':';
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

            if (char_belongs_to_token(text[i])) return;

            size_t start { i };
            while (i < LEN && std::isspace(text[i]) == 0
                   && !char_belongs_to_token(text[i]))
            {
                i++;
            }

            if (start < i)
            {
                std::string argument { text.substr(start, i - start) };

                tokens->add_token(TokenType::ARGUMENT, start, argument);
            }
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
                bool   after_eq { false };

                while (i + len < LEN && std::isspace(text[i + len]) == 0)
                {
                    if (!after_eq && char_belongs_to_token(text[i + len]))
                        break;
                    if (text[i + len] == '=') after_eq = true;

                    len++;
                }

                const std::string raw { text.substr(i, len) };
                const size_t      eq_pos { raw.find('=') };
                auto [flag, param] { utils::str::split(raw, eq_pos) };

                tokens->add_token(TokenType::FLAG, i, flag);
                if (eq_pos != std::string::npos)
                    tokens->add_token(TokenType::PARAMETER, i + eq_pos, param);

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

            if (text[i + len] == '=')
            {
                len++; /* for = */

                size_t flag_len { 0 };
                while (i + len + flag_len < LEN
                       && std::isspace(text[i + len + flag_len]) == 0)
                {
                    flag_len++;
                }

                std::string flag { text.substr(i + len, flag_len) };
                tokens->add_token(TokenType::PARAMETER, i + len, flag);
                i += flag_len;
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
            tokens->add_token(TokenType::COMMAND, 0, cmd);
        size_t i { tokens->tokens.back().get_data<std::string>()->length() };

        handle_argument(tokens, i, text);

        for (; i < text.length(); i++)
        {
            if (std::isspace(text[i]) != 0) continue;

            if (handle_string(tokens, i, text)) continue;
            if (handle_substitution(tokens, i, text)) continue;
            if (handle_flag(tokens, i, text)) continue;

            if (!char_belongs_to_token(text[i]))
            {
                size_t end_idx { i + 1 };
                while (end_idx < text.length()
                       && std::isspace(text[end_idx]) == 0
                       && !char_belongs_to_token(end_idx))
                    end_idx++;

                std::string param { text.substr(i, end_idx - i) };
                tokens->add_token(TokenType::PARAMETER, i, param);
                i += param.length();
            }
        }

        return tokens;
    }
}
