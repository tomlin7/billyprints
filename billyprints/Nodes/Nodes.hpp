#pragma once

#include "Node.hpp"
#include "Source.hpp"
#include "AND.hpp"
#include "OR.hpp"
#include "NOT.hpp"

namespace Billyprints {
    std::vector<Node* (*)()> availableNodes{
            []() -> Node* {
                return new Source();
            },
            []() -> Node* {
                return new AND();
            },
            []() -> Node* {
                return new OR();
            },
            []() -> Node* {
                return new NOT();
            }
    };
}