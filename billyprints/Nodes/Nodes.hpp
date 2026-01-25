#pragma once

#include "Node.hpp"

#include "AND.hpp"
#include "NOT.hpp"
#include "PinIn.hpp"
#include "PinOut.hpp"


namespace Billyprints {
extern std::vector<Node *(*)()> availableNodes;
}