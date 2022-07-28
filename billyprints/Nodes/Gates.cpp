#include "Gates.hpp"

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
            return new Buffer();
        },
        []() -> Gate* {
            return new NAND();
        },
        []() -> Gate* {
            return new NOR();
        },
        []() -> Gate* {
            return new XOR();
        },
        []() -> Gate* {
            return new XNOR();
        },
    };
}