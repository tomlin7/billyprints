#pragma once

#include "Node.hpp"

#include "Source.hpp"
#include "AND.hpp"
#include "OR.hpp"
#include "NOT.hpp"

namespace Billyprints {
    extern std::vector<Node* (*)()> availableNodes;
}