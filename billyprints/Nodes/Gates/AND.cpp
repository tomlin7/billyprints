#include "AND.hpp"
#include <iostream>

namespace Billyprints {
AND::AND() : Gate("AND", {{"a"}, {"b"}}, {{"out"}}) {}

bool AND::AND_F(const std::vector<bool> &input, const int &pinCount) {
  if (input.size() < pinCount)
    return false;

  bool current = input[0];
  for (const bool &pin : input)
    current &= pin;

  return current;
}

bool AND::Evaluate() {
  if (isEvaluating || lastEvaluatedFrame == GlobalFrameCount)
    return value;

  isEvaluating = true;
  std::vector<bool> input;
  for (const auto &cn : connections)
    if (cn.inputNode == this)
      input.push_back(((Node *)cn.outputNode)->Evaluate());

  value = AND_F(input, inputSlotCount);
  lastEvaluatedFrame = GlobalFrameCount;
  isEvaluating = false;
  return value;
}
} // namespace Billyprints
