#pragma once

#include "Gate.hpp"

#include "AND.hpp"
#include "Buffer.hpp"
#include "CustomGate.hpp"
#include "NAND.hpp"
#include "NOR.hpp"
#include "NOT.hpp"
#include "OR.hpp"
#include "XNOR.hpp"
#include "XOR.hpp"

#include <functional>

namespace Billyprints {
extern std::vector<std::function<Gate *()>> availableGates;
}