#pragma once
#include <string>

namespace Billyprints {
class Connection {
public:
  void *inputNode = nullptr;
  std::string inputSlot;

  void *outputNode = nullptr;
  std::string outputSlot;

  bool operator==(const Connection &other) const;
  bool operator!=(const Connection &other) const;
};
} // namespace Billyprints
