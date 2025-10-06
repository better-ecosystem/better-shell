#include "parser/parser.hh"
#include "utils.hh"

using namespace std::literals;


namespace parser
{
    namespace
    {
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
        handle_argument(std::vector<Token> &tokens,
                        size_t             &i,
                        const std::string  &text)
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

                std::string arg { text.substr(start, i - start) };
                tokens.emplace_back(TokenType::ARGUMENT, std::move(arg),
                                    std::nullopt);

                i++;
                return;
            }

            size_t start { i };
            while (i < LEN && std::isspace(text[i]) == 0 && text[i] != '$') ++i;

            if (start < i)
            {
                std::string arg { text.substr(start, i - start) };
                tokens.emplace_back(parser::TokenType::ARGUMENT, std::move(arg),
                                    std::nullopt);
            }
        }


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

            for (size_t i = start; i < text.size(); ++i)
            {
                if (text[i] == BracketKind::OPEN)
                    bracket_nest++;
                else if (text[i] == BracketKind::CLOSE)
                    bracket_nest--;
                if (bracket_nest == 0) return i;
            }

            return text.length();
        }


        [[nodiscard]]
        auto
        handle_substitution(std::vector<Token> &tokens,
                            size_t             &i,
                            const std::string  &text) -> bool
        {
            if (text[i] != '$' || i + 1 >= text.size() || text[i + 1] != '{')
                return false;

            tokens.emplace_back(TokenType::SUB_BRACKET, "${",
                                BracketKind::OPEN);
            i += 2;

            const size_t END_IDX { find_last_close_bracket(text, i) };
            std::string  inner { text.substr(i, END_IDX - i) };

            inner = utils::str::trim(inner);
            TokenGroup inner_tokens { parser::parse(inner) };

            tokens.emplace_back(TokenType::SUB_CONTENT, std::move(inner_tokens),
                                std::nullopt);
            i = END_IDX - 1;
            return true;
        }


        [[nodiscard]]
        auto
        handle_flag(std::vector<Token> &tokens,
                    size_t             &i,
                    const std::string  &text) -> bool
        {
            if (text[i] != '-' || std::isspace(text[i - 1]) == 0) return false;

            const size_t LEN { text.length() };

            /* Handles long arg */
            if (i + 1 < LEN && text[i + 1] == '-')
            {
                size_t len { 2 };
                while (i + len < LEN && std::isspace(text[i + len]) == 0) len++;
                tokens.emplace_back(TokenType::FLAG, text.substr(i, len),
                                    FlagKind::LONG);
                i += len - 1;
                return true;
            }

            /* Handle clustered short options: -abc -> -a, -b, -c */
            size_t len = 1;
            while (i + len < LEN && std::isspace(text[i + len]) == 0
                   && text[i + len] != '=')
            {
                tokens.emplace_back(TokenType::FLAG, "-"s + text[i + len],
                                    FlagKind::SHORT);
                len++;
            }

            i += len;
            return true;
        }
    }


    auto
    parse(const std::string &text) -> TokenGroup
    {
        std::vector<Token> tokens;
        tokens.emplace_back(TokenType::COMMAND, get_command(text),
                            std::nullopt);
        size_t i { tokens.back().get_data<std::string>().length() };

        handle_argument(tokens, i, text);

        for (; i < text.length(); i++)
        {
            if (std::isspace(text[i]) != 0) continue;

            if (handle_substitution(tokens, i, text)) continue;
            if (handle_flag(tokens, i, text)) continue;

            if (text[i] == '}')
            {
                tokens.emplace_back(TokenType::SUB_BRACKET, "}",
                                    BracketKind::CLOSE);
                continue;
            }
        }

        return { .tokens = std::move(tokens), .raw = text };
    }
}
