#pragma once

#include "Connection.hpp"
#include "Gates.hpp"
#include "Nodes.hpp"
#include <filesystem>
#include <set>

namespace Billyprints {
class NodeEditor {
  std::vector<Node *> nodes;
  char gateName[128] = "NewGate";
  float newGateColor[3] = {0.2f, 0.2f, 0.2f}; // Default color
  std::string debugMsg = "Ready";
  bool openCreateGatePopup = false;
  bool anyNodeDragged = false;

  void RenderNode(Node *node);
  void RenderNodes();
  void RenderContextMenu();
  void RenderDock();
  void RenderConnectionDropMenu();

  // Connection drop menu state
  bool showConnectionDropMenu = false;
  ImVec2 connectionDropPos;
  ImVec2 connectionSourceSlotPos;
  void *dropSourceNode = nullptr;
  std::string dropSourceSlot;
  int dropSourceSlotKind = 0;
  ImVec2 canvasWindowPos;
  void CreateGate();

  std::vector<GateDefinition> customGateDefinitions;
  void SaveGates(const std::string &filename);
  void LoadGates(const std::string &filename);

  // Scene save/load
  void SaveScene(const std::string &filename);
  void LoadScene(const std::string &filename);

  // Missing gate tracking (for custom gates not loaded)
  std::vector<std::string> missingGateTypes;
  std::set<PlaceholderGate *> placeholderNodes;
  bool showMissingGatesBanner = false;
  void TryUpgradePlaceholders();

  std::string editingGateName;
  std::string originalSceneScript;
  void DuplicateNode(Node *node);
  void UpdateGateDefinitionFromCurrentScene(const std::string &name);

  std::string currentScript;
  std::string lastParsedScript;
  std::string scriptError;
  std::string scriptDefinitions; // Stores define...end blocks for preservation
  bool showScriptEditor = true;
  bool errorPanelCollapsed = false;
  void UpdateScriptFromNodes();
  void UpdateNodesFromScript();

  bool openSaveGatePopup = false;
  bool openLoadGatePopup = false;
  bool openSaveScenePopup = false;
  bool openLoadScenePopup = false;
  char currentFilename[128] = "custom_gates.bin";
  char sceneFilename[128] = "scene.bps";
  std::filesystem::path currentPath = std::filesystem::current_path();

  bool showCodeEditor = false;
  std::string editingCode;
  Gate *gateBeingEdited = nullptr;
  bool showDock = true;

  void HandleKeyBindings();
  void SelectAllNodes();
  void DeselectAllNodes();
  void DuplicateSelectedNodes();
  void FrameSelectedNodes();
  void ResetView();

public:
  NodeEditor();
  void Redraw();
};
} // namespace Billyprints