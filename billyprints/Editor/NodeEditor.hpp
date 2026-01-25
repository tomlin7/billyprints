#pragma once

#include "Connection.hpp"
#include "Gates.hpp"
#include "Nodes.hpp"
#include <filesystem>

namespace Billyprints {
class NodeEditor {
  std::vector<Node *> nodes;
  char gateName[128] = "NewGate";
  float newGateColor[3] = {0.2f, 0.2f, 0.2f}; // Default color
  std::string debugMsg = "Ready";
  bool openCreateGatePopup = false;

  void RenderNode(Node *node);
  void RenderNodes();
  void RenderContextMenu();
  void CreateGate();

  std::vector<GateDefinition> customGateDefinitions;
  void SaveGates(const std::string &filename);
  void LoadGates(const std::string &filename);

  std::string currentScript;
  bool showScriptEditor = false;
  void UpdateScriptFromNodes();
  void UpdateNodesFromScript();

  bool openSaveGatePopup = false;
  bool openLoadGatePopup = false;
  char currentFilename[128] = "custom_gates.bin";
  std::filesystem::path currentPath = std::filesystem::current_path();

public:
  NodeEditor();
  void Redraw();
};
} // namespace Billyprints