#include "Gates.hpp"

namespace Billyprints {
std::vector<std::function<Gate *()>> availableGates{
    []() -> Gate * { return new AND(); },
    []() -> Gate * { return new NOT(); },
};
} // namespace Billyprints