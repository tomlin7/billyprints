#include "NodeEditor.hpp"
#include <string>
#include <vector>

namespace Billyprints {
inline void NodeEditor::RenderNode(Node *node) { node->Render(); }

void NodeEditor::RenderDock() {
  float dockHeight = 84.0f;
  float iconSize = 48.0f;
  float iconPadding = 20.0f;

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImVec2 workPos = viewport->WorkPos;
  ImVec2 workSize = viewport->WorkSize;

  size_t totalIcons = availableNodes.size() + availableGates.size();
  float totalWidth = totalIcons * (iconSize + iconPadding) + iconPadding;

  ImVec2 dockPos = ImVec2(workPos.x + (workSize.x - totalWidth) * 0.5f,
                          workPos.y + workSize.y - dockHeight - 30.0f);
  ImVec2 dockSize = ImVec2(totalWidth, dockHeight);

  // Use Foreground draw list to overlay everything
  ImDrawList *drawList = ImGui::GetForegroundDrawList();

  // Background Shadow/Glow
  drawList->AddRectFilled(
      ImVec2(dockPos.x - 5, dockPos.y - 5),
      ImVec2(dockPos.x + dockSize.x + 5, dockPos.y + dockSize.y + 5),
      IM_COL32(0, 0, 0, 60), 16.0f);

  // Glassmorphic Body
  drawList->AddRectFilled(
      dockPos, ImVec2(dockPos.x + dockSize.x, dockPos.y + dockSize.y),
      IM_COL32(30, 35, 45, 180), 14.0f);
  drawList->AddRect(dockPos,
                    ImVec2(dockPos.x + dockSize.x, dockPos.y + dockSize.y),
                    IM_COL32(255, 255, 255, 40), 14.0f, 0, 2.0f);

  float xOffset = iconPadding;
  ImVec2 mousePos = ImGui::GetMousePos();

  auto renderIcon = [&](const char *label, std::function<Node *()> factory,
                        ImU32 color) {
    ImVec2 center = ImVec2(dockPos.x + xOffset + iconSize * 0.5f,
                           dockPos.y + dockHeight * 0.5f);
    bool hovered = (mousePos.x >= dockPos.x + xOffset &&
                    mousePos.x <= dockPos.x + xOffset + iconSize &&
                    mousePos.y >= dockPos.y + (dockHeight - iconSize) * 0.5f &&
                    mousePos.y <= dockPos.y + (dockHeight + iconSize) * 0.5f);

    float scale = hovered ? 1.25f : 1.0f;
    float lift = hovered ? -12.0f : 0.0f;
    float currentSize = iconSize * scale;

    ImVec2 animatedCenter = ImVec2(center.x, center.y + lift);

    // Icon Circle
    drawList->AddCircleFilled(animatedCenter, currentSize * 0.5f,
                              (color & 0x00FFFFFF) | 0xDD000000, 32);
    drawList->AddCircle(animatedCenter, currentSize * 0.5f,
                        IM_COL32(255, 255, 255, 80), 32, 1.5f);

    // Symbol/Label in center
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font
    std::string shortLabel = label;
    if (shortLabel.size() > 4)
      shortLabel = shortLabel.substr(0, 3) + ".";

    float fontSize = 14.0f * scale;
    ImVec2 textSize = ImGui::CalcTextSize(shortLabel.c_str());
    drawList->AddText(
        NULL, fontSize,
        ImVec2(animatedCenter.x -
                   textSize.x * 0.5f * (fontSize / ImGui::GetFontSize()),
               animatedCenter.y -
                   textSize.y * 0.5f * (fontSize / ImGui::GetFontSize())),
        IM_COL32(255, 255, 255, 255), shortLabel.c_str());
    ImGui::PopFont();

    // Full Label Tooltip or hint
    if (hovered) {
      ImGui::BeginTooltip();
      ImGui::Text("%s", label);
      ImGui::EndTooltip();

      if (ImGui::IsMouseClicked(0)) {
        Node *newNode = factory();
        EditorTab *tab = GetActiveTab();
        if (tab) {
          tab->nodes.push_back(newNode);
          ImNodes::AutoPositionNode(newNode);
        }
      }
    }

    xOffset += iconSize + iconPadding;
  };

  for (auto &f : availableNodes) {
    Node *tmp = f();
    renderIcon(tmp->title, f, tmp->GetColor());
    delete tmp;
  }

  for (auto &f : availableGates) {
    Node *tmp = f();
    renderIcon(tmp->title, f, tmp->GetColor());
    delete tmp;
  }
}

inline void NodeEditor::RenderNodes() {
  EditorTab *tab = GetActiveTab();
  if (!tab)
    return;

  for (auto it = tab->nodes.begin(); it != tab->nodes.end();) {
    Node *node = *it;

    if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete) &&
        ImGui::IsWindowFocused()) {
      for (auto &connection : node->connections) {
        if (connection.outputNode == node) {
          ((Node *)connection.inputNode)->DeleteConnection(connection);
        } else {
          ((Node *)connection.outputNode)->DeleteConnection(connection);
        }
      }
      node->connections.clear();

      delete node;
      it = tab->nodes.erase(it);
    } else {
      RenderNode(node);
      ++it;
    }
  }
}

void NodeEditor::CreateNewTab() {
  EditorTab tab;
  static int counter = 1;
  tab.title = "Untitled " + std::to_string(counter++);
  tabs.push_back(std::move(tab));
  activeTabIndex = (int)tabs.size() - 1;
}

void NodeEditor::CloseTab(int index) {
  if (index < 0 || index >= tabs.size())
    return;

  tabs.erase(tabs.begin() + index);
  if (tabs.empty()) {
    CreateNewTab();
  }
  if (activeTabIndex >= tabs.size()) {
    activeTabIndex = (int)tabs.size() - 1;
  }
}

inline void NodeEditor::RenderContextMenu() {
  if (ImGui::BeginPopupContextWindow("NodesContextMenu")) {
    for (const auto &desc : availableNodes) {
      auto item = desc();
      if (ImGui::MenuItem(item->title)) {
        EditorTab *tab = GetActiveTab();
        if (tab) {
          tab->nodes.push_back(item);
          ImNodes::AutoPositionNode(tab->nodes.back());
        }
      }
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Gates")) {
      for (const auto &desc : availableGates) {
        auto item = desc();
        if (ImGui::MenuItem(item->title)) {
          EditorTab *tab = GetActiveTab();
          if (tab) {
            tab->nodes.push_back(item);
            ImNodes::AutoPositionNode(tab->nodes.back());
          }
        }
      }
      ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Reset Zoom"))
      ImNodes::GetCurrentCanvas()->Zoom = 1;
    ImGui::EndPopup();
  }
}

void NodeEditor::Redraw() {
  Node::GlobalFrameCount++;

  EditorTab *activeTab = GetActiveTab();
  if (!activeTab) {
    CreateNewTab();
    activeTab = GetActiveTab();
  }

  if (!activeTab)
    return;

  ImNodes::Ez::SetContext(activeTab->context);

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
      ImGuiWindowFlags_MenuBar;

  // IMPORTANT: Everything UI related must be entirely inside this Begin/End
  // block to avoid crashes
  if (ImGui::Begin("Billyprints Editor", nullptr, window_flags)) {

    // --- Menu Bar ---
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open..", "Ctrl+O")) {
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save Custom Gates...")) {
          openSaveGatePopup = true;
        }
        if (ImGui::MenuItem("Load Custom Gates...")) {
          openLoadGatePopup = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("New Tab", "Ctrl+T")) {
          CreateNewTab();
        }
        if (ImGui::MenuItem("Close Tab", "Ctrl+W")) {
          CloseTab(activeTabIndex);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Create Gate..")) {
          openCreateGatePopup = true;
        }
        if (ImGui::MenuItem("Force Create Gate (Debug)")) {
          debugMsg = "Force Creating Gate...";
          CreateGate();
          debugMsg += " -> Done";
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene Script", NULL, &showScriptEditor);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    // --- Tab Bar ---
    if (ImGui::BeginTabBar("MyTabs", ImGuiTabBarFlags_AutoSelectNewTabs)) {
      if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing |
                                        ImGuiTabItemFlags_NoTooltip)) {
        CreateNewTab();
      }

      for (int i = 0; i < tabs.size(); ++i) {
        bool open = true;
        std::string name = tabs[i].title + "###Tab" + std::to_string(i);

        ImGuiTabItemFlags flags = 0;
        if (tabs[i].unsaved)
          flags |= ImGuiTabItemFlags_UnsavedDocument;
        // Logic to programmatically select new tabs could go here if we tracked
        // "just created" state But for now, let ImGui handle selection.

        if (ImGui::BeginTabItem(name.c_str(), &open, flags)) {
          if (activeTabIndex != i) {
            activeTabIndex = i;
            // Context set happens at top of loop next proper redraw or
            // immediately if we want
            ImNodes::Ez::SetContext(tabs[i].context);
          }
          ImGui::EndTabItem();
        }

        if (!open) {
          CloseTab(i);
          // Adjust index if we closed the current or a previous tab
          if (i <= activeTabIndex)
            i--;
          // activeTab might be invalid now, break or handle
          //  But CloseTab handles re-creating logic if empty
          break;
        }
      }
      ImGui::EndTabBar();
    }

    // Re-acquire active tab after potential tab close/switch
    activeTab = GetActiveTab();
    if (activeTab) {
      ImNodes::Ez::SetContext(activeTab->context);

      // --- File Picker & Popups ---
      auto RenderFilePicker = [&]() {
        ImGui::TextWrapped("Path: %s",
                           activeTab->filepath.empty()
                               ? "Unsaved"
                               : activeTab->filepath.string().c_str());
        static std::filesystem::path browsingPath =
            std::filesystem::current_path();

        ImGui::TextWrapped("Browsing: %s", browsingPath.string().c_str());
        if (ImGui::Button("Up")) {
          if (browsingPath.has_parent_path())
            browsingPath = browsingPath.parent_path();
        }

        ImGui::Separator();
        ImGui::BeginChild("FileList", ImVec2(400, 200), true);

        try {
          for (const auto &entry :
               std::filesystem::directory_iterator(browsingPath)) {
            if (entry.is_directory()) {
              if (ImGui::Selectable(
                      ("[Dir] " + entry.path().filename().string()).c_str())) {
                browsingPath = entry.path();
              }
            } else {
              if (ImGui::Selectable(entry.path().filename().string().c_str())) {
                strcpy(currentFilename,
                       entry.path().filename().string().c_str());
              }
            }
          }
        } catch (...) {
        }
        ImGui::EndChild();
      };

      if (openSaveGatePopup)
        ImGui::OpenPopup("SaveGatePopup");
      if (ImGui::BeginPopupModal("SaveGatePopup", NULL,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", currentFilename, 128);
        RenderFilePicker();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
          SaveGates((currentPath / currentFilename).string());
          ImGui::CloseCurrentPopup();
          openSaveGatePopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
          ImGui::CloseCurrentPopup();
          openSaveGatePopup = false;
        }
        ImGui::EndPopup();
      }

      if (openLoadGatePopup)
        ImGui::OpenPopup("LoadGatePopup");
      if (ImGui::BeginPopupModal("LoadGatePopup", NULL,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", currentFilename, 128);
        RenderFilePicker();
        if (ImGui::Button("Load", ImVec2(120, 0))) {
          LoadGates((currentPath / currentFilename).string());
          ImGui::CloseCurrentPopup();
          openLoadGatePopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
          ImGui::CloseCurrentPopup();
          openLoadGatePopup = false;
        }
        ImGui::EndPopup();
      }

      if (openCreateGatePopup)
        ImGui::OpenPopup("CreateGatePopup");
      if (ImGui::BeginPopupModal("CreateGatePopup", NULL,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Gate Name", gateName, 128);
        ImGui::ColorEdit3("Color", newGateColor);
        if (ImGui::Button("Create", ImVec2(120, 0))) {
          CreateGate();
          ImGui::CloseCurrentPopup();
          openCreateGatePopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          ImGui::CloseCurrentPopup();
          openCreateGatePopup = false;
        }
        ImGui::EndPopup();
      }

      // --- Editor Split ---
      float sidebarWidth = 400.0f;
      if (showScriptEditor) {
        ImGui::Columns(2, "EditorSplit", true);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - sidebarWidth);
      }

      // --- Canvas ---
      ImNodes::Ez::BeginCanvas();
      RenderNodes();

      // --- Visuals & Overlays (Inside Canvas for context access) ---
      auto *canvas = ImNodes::GetCurrentCanvas();
      if (canvas) {
        ImNodes::Ez::PushStyleVar(ImNodesStyleVar_NodeRounding, 8.0f);
        canvas->Colors[ImNodes::ColCanvasLines] = IM_COL32(0, 0, 0, 0);

        RenderContextMenu();
        RenderDock();

        ImNodes::Ez::PopStyleVar();
      }

      ImNodes::Ez::EndCanvas();

      // --- Dock Overlay (Now Inside Canvas) ---
      // Reset cursor to draw overlay on top if needed?
      // RenderDock uses ForegroundDrawList so it doesn't matter where cursor is
      // usually.

      // --- Sidebar ---
      if (showScriptEditor) {
        ImGui::NextColumn();
        ImGui::BeginChild("ScriptPanel");

        // Script editor logic
        static char scriptBuf[1024 * 64];

        // Determine if we are editing the script
        ImGuiID scriptID = ImGui::GetID("##scene");
        bool isTyping = (ImGui::GetActiveID() == scriptID);

        if (!isTyping) {
          // Not typing: Sync Nodes -> Script (Live)
          UpdateScriptFromNodes();
          strncpy(scriptBuf, activeTab->currentScript.c_str(),
                  sizeof(scriptBuf) - 1);
        }

        if (ImGui::InputTextMultiline("##scene", scriptBuf, sizeof(scriptBuf),
                                      ImVec2(-1, -1))) {
          activeTab->currentScript = scriptBuf;
        }

        // Sync Script -> Nodes when done editing (or maybe on change?)
        // DeactivatedAfterEdit is good to avoid constant rebuilding while
        // typing
        if (ImGui::IsItemDeactivatedAfterEdit()) {
          UpdateNodesFromScript();
        }
        if (!activeTab->scriptError.empty()) {
          ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s",
                             activeTab->scriptError.c_str());
        }
        ImGui::EndChild();
      }
    }
  }
  ImGui::End();
}

NodeEditor::NodeEditor() { CreateNewTab(); }
} // namespace Billyprints
