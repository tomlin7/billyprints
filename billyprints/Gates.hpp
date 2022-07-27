#pragma once

#include "Node.hpp"
#include "AND.hpp"
#include "OR.hpp"
#include "NOT.hpp"
#include "NAND.hpp"
#include "NOR.hpp"

namespace Billyprints {
    std::vector<Gate* (*)()> availableGates{
        []() -> Gate* {
            return new AND();
        },
        []() -> Gate* {
            return new OR();
        },
        []() -> Gate* {
            return new NOT();
        },
        []() -> Gate* {
            return new NAND();
        },
        []() -> Gate* {
            return new NOR();
        },
    };
}