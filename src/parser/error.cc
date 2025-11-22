#include "parser/error.hh"

using namespace parser;


auto
parser::error::compute_real_index(const TokenGroup *group, const Token *tkn)
    -> std::size_t
{
    std::size_t real_idx { tkn->index };

    const TokenGroup *current { group };

    while (auto parent_ptr { current->parent.lock() })
    {
        std::size_t child_idx { 0 };
        for (; child_idx < parent_ptr->tokens.size(); child_idx++)
        {
            auto &t { parent_ptr->tokens[child_idx] };
            if (t.type == TokenType::SUB_CONTENT)
                if (const auto *sub { t.get_data<shared_tokens>() })
                    if (sub->get() == current) break;
        }

        std::size_t offset { 0 };
        for (std::size_t i { 0 }; i < child_idx; i++)
        {
            auto &t { parent_ptr->tokens[i] };
            if (t.type == TokenType::SUB_CONTENT)
            {
                if (const auto *sub { t.get_data<shared_tokens>() })
                    offset += sub->get()->raw.length();
            }
            else if (const auto *str_ptr { t.get_data<std::string>() })
                offset += str_ptr->length();
        }

        real_idx += offset;
        real_idx++;

        current = parent_ptr.get();
    }

    return real_idx;
}
