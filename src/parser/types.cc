#include <utility>

#include "parser/error.hh"
#include "parser/types.hh"


namespace parser
{
    auto
    TokenGroup::get_highlighted() const -> std::string
    {
        std::string result;
        bool        first { true };

        for (const auto &token : this->tokens)
        {
            if (!first) result += " ";

            result += token.get_highlighted() + " ";
            first   = false;
        }

        return result;
    }


    auto
    TokenGroup::to_json() -> Json::Value
    {
        Json::Value root { Json::objectValue };

        root["raw"]    = this->raw;
        root["tokens"] = Json::arrayValue;

        for (auto &token : this->tokens)
        {
            Json::Value j_token { Json::objectValue };
            j_token["type"] = Json::String { TokenType_to_string(token.type) };

            j_token["data"]
                = token.type == TokenType::SUB_CONTENT
                    ? token.get_data<shared_tokens>()->get()->to_json()
                    : *token.get_data<std::string>();
            j_token["index"] = token.index;

            root["tokens"].append(j_token);
        }

        return root;
    }


    auto
    TokenGroup::get_toplevel() const -> const TokenGroup *
    {
        const TokenGroup *current { this };
        while (auto parent_ptr { current->parent.lock() })
            current = parent_ptr.get();

        return current;
    }


    TokenGroup::TokenGroup(std::string raw, const shared_tokens &parent)
        : tokens({}), raw(std::move(raw)), source("stdin"), parent(parent)
    {
    }


    Token::Token(TokenType type, size_t idx, std::string data)
        : type(type), index(idx), data(std::move(data))
    {
    }


    Token::Token(TokenType type, size_t idx, const shared_tokens &data)
        : type(type), index(idx), data(data), operator_type(OperatorType::NONE)
    {
    }


    auto
    Token::operator==(const Token &other) const -> bool
    {
        return this->type == other.type && this->index == other.index
            && this->data == other.data
            && this->operator_type == other.operator_type;
    }


    auto
    Token::get_highlighted() const -> std::string
    {
        return "";
    }
}
