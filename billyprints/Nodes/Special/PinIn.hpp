#pragma once

#include "Node.hpp"

namespace Billyprints {
class PinIn : public Node {
public:
  PinIn();
  bool Evaluate() override;
  void Render() override;
  ImU32 GetColor() const override { return IM_COL32(20, 60, 20, 255); }
};
} // namespace Billyprints
