#pragma once

#include "Gate.hpp"

#include "AND.hpp"
#include "OR.hpp"
#include "NOT.hpp"
#include "NAND.hpp"
#include "NOR.hpp"

namespace Billyprints {
    extern std::vector<Gate* (*)()> availableGates;
}