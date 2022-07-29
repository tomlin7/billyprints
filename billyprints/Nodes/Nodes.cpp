#include "Nodes.hpp"

namespace Billyprints {
    std::vector<Node* (*)()> availableNodes {
        []() -> Node* {
            return new PinIn();
        },
        []() -> Node* {
            return new PinOut();
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