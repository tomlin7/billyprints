#pragma once

#include "Connection.hpp"
#include "Gates.hpp"
#include "Nodes.hpp"
#include <ImNodesEz.h>
#include <filesystem>

namespace Billyprints {
class NodeEditor {
  std::vector<GateDefinition> customGateDefinitions;
  char gateName[128] = "NewGate";
  float newGateColor[3] = {0.2f, 0.2f, 0.2f};
  std::string debugMsg = "Ready";
  bool openCreateGatePopup = false;
  bool showScriptEditor = true;
  bool errorPanelCollapsed = false;
  char currentFilename[128] = "custom_gates.bin";
  std::filesystem::path currentPath = std::filesystem::current_path();
  bool openSaveGatePopup = false;
  bool openLoadGatePopup = false;

  void RenderNode(Node *node);
  void RenderNodes();
  void RenderContextMenu();
  void RenderDock();
  void CreateGate();
  void SaveGates(const std::string &filename);
  void LoadGates(const std::string &filename);
  void UpdateScriptFromNodes();
  void UpdateNodesFromScript();

public:
  struct EditorTab {
    std::string title = "Untitled";
    std::filesystem::path filepath;
    bool unsaved = false;

    std::vector<Node *> nodes;
    ImNodes::Ez::Context *context = nullptr;

    std::string currentScript;
    std::string lastParsedScript;
    std::string scriptError;
    int autoIdCounter = 0;

    EditorTab() { context = ImNodes::Ez::CreateContext(); }
    ~EditorTab() {
      // Note: Nodes should be deleted by the editor/tab when closing
      for (auto *n : nodes)
        delete n;
      ImNodes::Ez::FreeContext(context);
    }

    // Disable copy to prevent double-free of context
    EditorTab(const EditorTab &) = delete;
    // Enable move
    EditorTab(EditorTab &&other) noexcept {
      // Initialize with other's specific data
      *this = std::move(other);
    }

    EditorTab &operator=(EditorTab &&other) noexcept {
      if (this != &other) {
        // Free current context if exists
        if (context) {
          ImNodes::Ez::FreeContext(context);
        }
        for (auto *n : nodes)
          delete n;
        nodes.clear();

        title = std::move(other.title);
        filepath = std::move(other.filepath);
        unsaved = other.unsaved;
        nodes = std::move(other.nodes);
        context = other.context;
        other.context = nullptr;
        currentScript = std::move(other.currentScript);
        lastParsedScript = std::move(other.lastParsedScript);
        scriptError = std::move(other.scriptError);
        autoIdCounter = other.autoIdCounter;
      }
      return *this;
    }
  };

private:
  std::vector<EditorTab> tabs;
  int activeTabIndex = -1;

  EditorTab *GetActiveTab() {
    if (activeTabIndex >= 0 && activeTabIndex < tabs.size())
      return &tabs[activeTabIndex];
    return nullptr;
  }

  void CreateNewTab();
  void CloseTab(int index);

public:
  NodeEditor();
  void Redraw();
};
} // namespace Billyprints