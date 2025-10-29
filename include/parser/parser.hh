#pragma once
#include "parser/types.hh"


namespace parser
{
    /**
     * parses text into a bunch of tokens, this function will never throw
     * ------------------------------------------------------------------
     *
     * @note non-token word are words that will not become a token on their own
     *
     *  - TokenType::COMMAND
     *      the parser will get the first word of each sentence, and set it
     *    as the command
     *
     *  - TokenType::ARGUMENT
     *      the parser will get the word next to the command as the argument
     *    if the word is a non-token word
     *
     *  - TokenType::FLAG
     *      the parser will treat all word with a dash starting the word
     *    as a flag, the parser will also seperate clumped short flags for
     *    example, it will seperate -sF into -s and -F
     *
     *  - TokenType::PARAMETER
     *      the parser will treat all non-token word that comes after a
     *    flag or an argument a parameter, and the parser will also seperate
     *    an argument and a flag with an equal sign in them to their respective
     *    type and parameter
     *
     *  - TokenType::OPERATOR
     *      the parser will treat any OperatorType characters not inside a
     *    string an operator
     *
     *  - TokenType::SUB_BRACKET (substitution-bracket)
     *      the parser will treat all '{' and '}' not inside a string as
     *    a substitution bracket
     *
     *  - TokenType::SUB_CONTENT (substitution-content)
     *      the parser will treat any sentence after a '{' and before a '}' as
     *    a substitution content
     *
     *  - TokenType::STRING_QUOTE
     *      the parser will treat all '"" not escaped as a string quote
     *
     *  - TokenType::STRING_CONTENT
     *      the parser will treat any sentence after a '"' and before a second
     *    '"' as a string content
     */
    [[nodiscard]]
    auto parse(std::string          input_source,
               const std::string   &text,
               const shared_tokens &parent = nullptr) noexcept -> shared_tokens;
}
