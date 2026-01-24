#pragma once

#include "Connection.hpp"
#include "Gates.hpp"
#include "Nodes.hpp"

namespace Billyprints {
class NodeEditor {
  std::vector<Node *> nodes;
  char gateName[128] = "NewGate";
  std::string debugMsg = "Ready";
  bool openCreateGatePopup = false;

  void RenderNode(Node *node);
  void RenderNodes();
  void RenderContextMenu();
  void CreateGate();

public:
  NodeEditor();
  void Redraw();
};
} // namespace Billyprints