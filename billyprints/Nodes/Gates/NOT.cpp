#include "NOT.hpp"

namespace Billyprints {
NOT::NOT() : Gate("NOT", {{"in"}}, {{"out"}}) {}

bool NOT::NOT_F(const std::vector<bool> &input, const int &) {
  for (const bool &pin : input)
    if (!pin)
      return !pin;

  return false;
}

bool NOT::Evaluate() {
  if (isEvaluating || lastEvaluatedFrame == GlobalFrameCount)
    return value;

  isEvaluating = true;
  std::vector<bool> input;
  for (const auto &cn : connections)
    if (cn.inputNode == this)
      input.push_back(((Node *)cn.outputNode)->Evaluate());

  value = NOT_F(input, inputSlotCount);
  lastEvaluatedFrame = GlobalFrameCount;
  isEvaluating = false;
  return value;
}
} // namespace Billyprints
