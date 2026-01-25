#include "PinIn.hpp"

namespace Billyprints {
PinIn::PinIn() : Node("In", {}, {{"->"}}) { value = true; };

bool PinIn::Evaluate() { return value; };
} // namespace Billyprints
