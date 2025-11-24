#pragma once
#include "parser/types.hh"


namespace parser
{
    [[nodiscard]]
    auto parse(std::string          input_source,
               const std::string   &text,
               const shared_tokens &parent = nullptr) noexcept -> shared_tokens;
}
