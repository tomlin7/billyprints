#include "NodeEditor.hpp"
#include <string>
#include <vector>

namespace Billyprints {
inline void NodeEditor::RenderNode(Node *node) { node->Render(); }

// Global pointers for interaction helpers
Node *nodeToDuplicate = nullptr;
Node *nodeToEdit = nullptr;
Node *nodeToDelete = nullptr;
bool nodeHoveredForContextMenu = false;

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
        nodes.push_back(newNode);
        ImNodes::AutoPositionNode(newNode);
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

void NodeEditor::DuplicateNode(Node *node) {
  if (!node)
    return;
  Node *newNode = CreateNodeByType(node->title);
  if (newNode) {
    newNode->pos = ImVec2(node->pos.x + 30.0f, node->pos.y + 30.0f);
    nodes.push_back(newNode);
    ImNodes::AutoPositionNode(newNode);
  }
}

inline void NodeEditor::RenderNodes() {
  for (auto it = nodes.begin(); it != nodes.end();) {
    Node *node = *it;

    RenderNode(node);

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
      it = nodes.erase(it);
    } else
      ++it;
  }

  // Handle global interaction requests
  if (nodeToDuplicate) {
    DuplicateNode(nodeToDuplicate);
    nodeToDuplicate = nullptr;
  }
  if (nodeToEdit) {
    originalSceneScript = currentScript;
    // For now, we'll just show the banner.
    // In a full implementation, we'd load the gate's script here.
    editingGateName = nodeToEdit->title;
    nodeToEdit = nullptr;
  }
  if (nodeToDelete) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
      if (*it == nodeToDelete) {
        // Handle connections
        for (auto &connection : (*it)->connections) {
          if (connection.outputNode == *it) {
            ((Node *)connection.inputNode)->DeleteConnection(connection);
          } else {
            ((Node *)connection.outputNode)->DeleteConnection(connection);
          }
        }
        delete *it;
        nodes.erase(it);
        break;
      }
    }
    nodeToDelete = nullptr;
  }
}

inline void NodeEditor::RenderContextMenu() {
  if (!nodeHoveredForContextMenu &&
      ImGui::BeginPopupContextWindow("NodesContextMenu",
                                     ImGuiPopupFlags_MouseButtonRight |
                                         ImGuiPopupFlags_NoOpenOverItems)) {
    for (const auto &desc : availableNodes) {
      auto item = desc();
      if (ImGui::MenuItem(item->title)) {
        nodes.push_back(item);
        ImNodes::AutoPositionNode(nodes.back());
      }
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Gates")) {
      for (const auto &desc : availableGates) {
        auto item = desc();
        if (ImGui::MenuItem(item->title)) {
          nodes.push_back(item);
          ImNodes::AutoPositionNode(nodes.back());
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
  nodeHoveredForContextMenu = false;
  Node::GlobalFrameCount++;
  auto context = ImNodes::Ez::CreateContext();
  IM_UNUSED(context);

  // --- REMOVED REDUNDANT MENU BAR BLOCK ---

  // File Picker Helper
  auto RenderFilePicker = [&]() {
    ImGui::TextWrapped("Path: %s", currentPath.string().c_str());
    if (ImGui::Button("Up")) {
      if (currentPath.has_parent_path())
        currentPath = currentPath.parent_path();
    }

    ImGui::Separator();
    ImGui::BeginChild("FileList", ImVec2(400, 200), true);

    std::vector<std::filesystem::directory_entry> dirs, files;
    try {
      for (const auto &entry :
           std::filesystem::directory_iterator(currentPath)) {
        if (entry.is_directory())
          dirs.push_back(entry);
        else if (entry.path().extension() == ".bin")
          files.push_back(entry);
      }
    } catch (...) {
    }

    for (const auto &dir : dirs) {
      std::string dirName = "[DIR] " + dir.path().filename().string();
      if (ImGui::Selectable(dirName.c_str())) {
        currentPath = dir.path();
      }
    }

    for (const auto &file : files) {
      if (ImGui::Selectable(file.path().filename().string().c_str())) {
        strncpy(currentFilename, file.path().filename().string().c_str(), 128);
      }
    }

    ImGui::EndChild();
  };

  // Save Gate Popup
  if (openSaveGatePopup) {
    ImGui::OpenPopup("SaveGatePopup");
    openSaveGatePopup = false;
  }
  if (ImGui::BeginPopupModal("SaveGatePopup", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Filename", currentFilename, 128);

    RenderFilePicker();
    ImGui::Separator();

    if (ImGui::Button("Save", ImVec2(120, 0))) {
      std::filesystem::path fullPath = currentPath / currentFilename;
      SaveGates(fullPath.string());
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  // Load Gate Popup
  if (openLoadGatePopup) {
    ImGui::OpenPopup("LoadGatePopup");
    openLoadGatePopup = false;
  }
  if (ImGui::BeginPopupModal("LoadGatePopup", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Filename", currentFilename, 128);

    RenderFilePicker();
    ImGui::Separator();

    if (ImGui::Button("Load", ImVec2(120, 0))) {
      std::filesystem::path fullPath = currentPath / currentFilename;
      LoadGates(fullPath.string());
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus;

  ImVec2 mainWinPos;
  float mainWinWidth;

  if (ImGui::Begin("Billyprints", NULL, window_flags)) {
    mainWinPos = ImGui::GetWindowPos();
    mainWinWidth = ImGui::GetWindowWidth();

    // Editing Banner
    if (!editingGateName.empty()) {
      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(180, 100, 20, 200));
      ImGui::BeginChild("EditingBanner", ImVec2(0, 40), true);
      ImGui::Text("Editing Gate: %s", editingGateName.c_str());
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - 160);
      if (ImGui::Button("Save and Close", ImVec2(100, 0))) {
        // Find the gate definition and update it
        for (auto &def : customGateDefinitions) {
          if (def.name == editingGateName) {
            UpdateScriptFromNodes();
            // We need a way to parse back to GateDefinition,
            // but for now, we'll just stop editing.
            // Ideally we'd re-generate the definition here.
            break;
          }
        }
        currentScript = originalSceneScript;
        UpdateNodesFromScript();
        editingGateName = "";
      }
      ImGui::SameLine();
      if (ImGui::Button("Discard", ImVec2(60, 0))) {
        currentScript = originalSceneScript;
        UpdateNodesFromScript();
        editingGateName = "";
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();
    }

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
        if (ImGui::MenuItem("Create Gate..")) {
          openCreateGatePopup = true;
        }
        if (ImGui::MenuItem("Close", "Ctrl+W")) {
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

    if (openCreateGatePopup) {
      ImGui::OpenPopup("CreateGatePopup");
      openCreateGatePopup = false;
    }

    if (ImGui::BeginPopupModal("CreateGatePopup", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::InputText("Gate Name", gateName, 128);
      ImGui::ColorEdit3("Color", newGateColor);
      if (ImGui::Button("Create", ImVec2(120, 0))) {
        debugMsg = "Creating Gate: " + std::string(gateName);
        CreateGate();
        debugMsg += " -> Done";
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  float sidebarWidth = 400.0f;
  if (showScriptEditor) {
    ImGui::Columns(2, "EditorSplit", true);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - sidebarWidth);
  }

  ImNodes::Ez::BeginCanvas();
  RenderDock();

  // --- Cyberpunk Visuals Start ---

  // 1. Hex Grid Background
  auto *canvas = ImNodes::GetCurrentCanvas();
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImVec2 canvasPos = ImGui::GetCursorScreenPos();
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();

  // Draw deep dark background
  drawList->AddRectFilled(
      canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
      IM_COL32(20, 20, 25, 255));

  // Grid Logic
  const float HEX_SIZE = 50.0f * canvas->Zoom;
  const float SQRT3 = 1.7320508f;
  const float HEX_W = HEX_SIZE * SQRT3;
  const float HEX_H = HEX_SIZE * 2.0f;
  const float HEX_X_SPACING = HEX_W;
  const float HEX_Y_SPACING = HEX_H * 0.75f;

  ImVec2 offset = canvas->Offset;

  // Calculate visible range
  int startX = (int)((-offset.x) / HEX_X_SPACING) - 1;
  int startY = (int)((-offset.y) / HEX_Y_SPACING) - 1;
  int endX = startX + (int)(canvasSize.x / HEX_X_SPACING) + 2;
  int endY = startY + (int)(canvasSize.y / HEX_Y_SPACING) + 2;

  ImColor gridColor = IM_COL32(50, 60, 70, 40); // Faint hex lines

  for (int y = startY; y < endY; ++y) {
    for (int x = startX; x < endX; ++x) {
      float xPos = x * HEX_X_SPACING;
      float yPos = y * HEX_Y_SPACING;
      if (y % 2 != 0)
        xPos += HEX_X_SPACING * 0.5f;

      ImVec2 center =
          ImVec2(canvasPos.x + offset.x + xPos, canvasPos.y + offset.y + yPos);

      // Draw Hexagon
      ImVec2 verts[6];
      for (int i = 0; i < 6; ++i) {
        float angle = 3.14159f / 3.0f * (i + 0.5f);
        verts[i] = ImVec2(center.x + cos(angle) * HEX_SIZE * 0.5f,
                          center.y + sin(angle) * HEX_SIZE * 0.5f);
      }
      drawList->AddPolyline(verts, 6, gridColor, true, 1.5f);
    }
  }

  // 2. Glassmorphism Styles
  ImNodes::Ez::PushStyleVar(ImNodesStyleVar_NodeRounding, 8.0f);
  ImNodes::Ez::PushStyleVar(ImNodesStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));
  ImNodes::Ez::PushStyleVar(ImNodesStyleVar_NodeSpacing, ImVec2(4.0f, 4.0f));

  // Update rendering colors in RenderNode manually? No, we set them per node
  // locally mostly, but lets set defaults for others
  canvas->Colors[ImNodes::ColCanvasLines] =
      IM_COL32(0, 0, 0, 0); // Hide default grid
  canvas->Colors[ImNodes::ColNodeBorder] =
      IM_COL32(100, 200, 255, 150); // Cyan glass border

  ImGui::Text("Debug: %s", debugMsg.c_str());

  RenderNodes();

  RenderContextMenu();

  ImNodes::Ez::EndCanvas();
  ImNodes::Ez::PopStyleVar(3);

  if (showScriptEditor) {
    ImGui::NextColumn();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginChild("ScriptPanel", ImVec2(0, 0), false);

    // Header Row
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::Text("Scene Script");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 90);
    if (ImGui::SmallButton("Sync")) {
      UpdateScriptFromNodes();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Apply")) {
      UpdateNodesFromScript();
    }
    ImGui::PopStyleVar(); // FramePadding

    // Only update script from nodes if the user isn't currently typing in the
    // editor
    static bool scriptActive = false;

    if (!scriptActive) {
      UpdateScriptFromNodes();
    }

    static char scriptBuf[8192];
    static bool bufferInitialized = false;
    if (!scriptActive || !bufferInitialized) {
      strncpy(scriptBuf, currentScript.c_str(), 8191);
      bufferInitialized = true;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    if (ImGui::InputTextMultiline(
            "##script", scriptBuf, 8192,
            ImVec2(-1, -ImGui::GetTextLineHeightWithSpacing() * 8))) {
      currentScript = scriptBuf;
      UpdateNodesFromScript();
    }
    ImGui::PopStyleVar();

    scriptActive = ImGui::IsItemActive();

    if (!scriptError.empty()) {
      ImGui::Separator();
      if (ImGui::Selectable(errorPanelCollapsed ? "> Show Errors"
                                                : "v Hide Errors")) {
        errorPanelCollapsed = !errorPanelCollapsed;
      }
      if (!errorPanelCollapsed) {
        ImGui::BeginChild("ErrorList", ImVec2(0, 150), true);
        ImGui::SetWindowFontScale(0.9f);
        ImGui::TextWrapped("%s", scriptError.c_str());
        ImGui::EndChild();
      }
    } // End of !scriptError.empty()
    ImGui::EndChild();
    ImGui::PopStyleVar(2); // ItemSpacing, WindowPadding
    ImGui::Columns(1);
  }

  ImGui::End();

  // Persistent Sidebar Toggle Overlay
  float buttonWidth = 30.0f;
  float overlayX = mainWinPos.x + mainWinWidth - buttonWidth -
                   5; // Default (Collapsed) position

  if (showScriptEditor) {
    overlayX = mainWinPos.x + mainWinWidth - sidebarWidth - buttonWidth;
  }

  ImGui::SetNextWindowPos(ImVec2(overlayX, mainWinPos.y + 300));
  ImGui::SetNextWindowBgAlpha(0.0f);
  if (ImGui::Begin(
          "SidebarToggleOverlay", NULL,
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
              ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
    if (ImGui::Button(showScriptEditor ? ">" : "<", ImVec2(buttonWidth, 30))) {
      showScriptEditor = !showScriptEditor;
    }
  }
  ImGui::End();
}

NodeEditor::NodeEditor() {}
} // namespace Billyprints
