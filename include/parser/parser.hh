#pragma once
#include "parser/types.hh"


namespace parser
{
    [[nodiscard]]
    auto parse(const std::string &text) -> TokenGroup;
}