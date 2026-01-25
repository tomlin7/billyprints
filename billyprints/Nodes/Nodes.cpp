#include "Nodes.hpp"

namespace Billyprints {
std::vector<Node *(*)()> availableNodes{
    []() -> Node * { return new PinIn(); },
    []() -> Node * { return new PinOut(); }};
} // namespace Billyprints