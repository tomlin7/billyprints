#include "NodeEditor.hpp"
#include "../Nodes/Gates/CustomGate.hpp"
#include <ImNodes.h>
#include <filesystem>
#include <functional>
#include <imgui.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Billyprints {
inline void NodeEditor::RenderNode(Node *node) { node->Render(); }

void NodeEditor::UpdateScriptFromNodes() {
  std::stringstream ss;
  std::map<Node *, int> nodeToId;
  for (int i = 0; i < nodes.size(); ++i) {
    nodeToId[nodes[i]] = i;
    std::string type = nodes[i]->title;
    ss << type << " n" << i << " @ " << (int)nodes[i]->pos.x << ", "
       << (int)nodes[i]->pos.y;

    if (type == "In") {
      PinIn *pin = (PinIn *)nodes[i];
      if (pin->isMomentary)
        ss << " momentary";
    }

    ss << "\n";
  }
  ss << "\n";
  for (auto *node : nodes) {
    for (const auto &conn : node->connections) {
      if (conn.outputNode == node) {
        ss << "n" << nodeToId[(Node *)conn.outputNode] << "." << conn.outputSlot
           << " -> "
           << "n" << nodeToId[(Node *)conn.inputNode] << "." << conn.inputSlot
           << "\n";
      }
    }
  }
  currentScript = ss.str();
}

void NodeEditor::UpdateNodesFromScript() {
  if (currentScript == lastParsedScript)
    return;
  lastParsedScript = currentScript;
  scriptError = "";

  auto trim = [](std::string &s) {
    if (s.empty())
      return;
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    size_t last = s.find_last_not_of(" \t\n\r");
    if (last != std::string::npos)
      s.erase(last + 1);
  };

  for (auto *n : nodes)
    delete n;
  nodes.clear();

  std::stringstream ss(currentScript);
  std::string line;
  std::map<std::string, Node *> idToNode;
  int lineNum = 0;

  while (std::getline(ss, line)) {
    lineNum++;
    trim(line);
    if (line.empty() || (line.size() >= 2 && line[0] == '/' && line[1] == '/'))
      continue;

    try {
      if (line.find("->") != std::string::npos) {
        size_t arrowPos = line.find("->");
        std::string left = line.substr(0, arrowPos);
        std::string right = line.substr(arrowPos + 2);
        trim(left);
        trim(right);

        auto parseSlot =
            [&](std::string s,
                bool isOutput) -> std::pair<std::string, std::string> {
          size_t dot = s.find('.');
          if (dot == std::string::npos) {
            return {s, isOutput ? "out" : "in"};
          }
          std::string nodePart = s.substr(0, dot);
          std::string slotPart = s.substr(dot + 1);
          trim(nodePart);
          trim(slotPart);
          return {nodePart, slotPart};
        };

        auto outS = parseSlot(left, true);
        auto inS = parseSlot(right, false);

        if (outS.first.empty() || inS.first.empty() || outS.second.empty() ||
            inS.second.empty())
          continue;

        if (idToNode.count(outS.first) && idToNode.count(inS.first)) {
          Node *outNode = idToNode[outS.first];
          Node *inNode = idToNode[inS.first];

          bool outSlotValid = false;
          if (outS.second == "out" && std::string(outNode->title) == "In")
            outSlotValid = true;
          else {
            for (int i = 0; i < outNode->outputSlotCount; ++i)
              if (std::string(outNode->outputSlots[i].title) == outS.second)
                outSlotValid = true;
          }

          bool inSlotValid = false;
          if (inS.second == "in" && std::string(inNode->title) == "Out")
            inSlotValid = true;
          else {
            for (int i = 0; i < inNode->inputSlotCount; ++i)
              if (std::string(inNode->inputSlots[i].title) == inS.second)
                inSlotValid = true;
          }

          if (outSlotValid && inSlotValid) {
            Connection conn;
            conn.outputNode = outNode;
            conn.outputSlot = outS.second;
            conn.inputNode = inNode;
            conn.inputSlot = inS.second;
            outNode->connections.push_back(conn);
            inNode->connections.push_back(conn);
          }
        }
      } else if (line.find("@") != std::string::npos) {
        std::stringstream lss(line);
        std::string type, id, at;
        int x, y;
        char comma;
        if (!(lss >> type >> id >> at >> x >> comma >> y)) {
          scriptError +=
              "Line " + std::to_string(lineNum) + ": Invalid node format\n";
          continue;
        }

        Node *n = CreateNodeByType(type);
        if (n) {
          n->pos = {(float)x, (float)y};
          if (type == "In" && line.find("momentary") != std::string::npos) {
            ((PinIn *)n)->isMomentary = true;
          }
          nodes.push_back(n);
          idToNode[id] = n;
        } else {
          scriptError += "Line " + std::to_string(lineNum) + ": Unknown type " +
                         type + "\n";
        }
      }
    } catch (...) {
      scriptError += "Line " + std::to_string(lineNum) + ": Unexpected error\n";
    }
  }
}

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
}

inline void NodeEditor::RenderContextMenu() {
  if (ImGui::BeginPopupContextWindow("NodesContextMenu")) {
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

void NodeEditor::CreateGate() {
  GateDefinition def;
  def.name = std::string(gateName);

  std::map<Node *, int> nodePtrToId;
  int idCounter = 0;

  // 1. Collect Nodes
  for (auto *node : nodes) {
    NodeDefinition nd;
    nd.id = idCounter++;
    nodePtrToId[node] = nd.id;
    nd.pos = node->pos;

    nd.type = node->title;

    def.nodes.push_back(nd);

    std::string titleStr = node->title;
    if (titleStr == "Input" || titleStr == "In")
      def.inputPinIndices.push_back(nd.id);
    else if (titleStr == "Output" || titleStr == "Out")
      def.outputPinIndices.push_back(nd.id);
  }

  // 2. Collect Connections
  for (auto *node : nodes) {
    for (const auto &conn : node->connections) {
      if (conn.outputNode == node) {
        ConnectionDefinition cd;
        cd.inputNodeId = nodePtrToId[(Node *)conn.inputNode];
        cd.inputSlot = conn.inputSlot;
        cd.outputNodeId = nodePtrToId[(Node *)conn.outputNode];
        cd.outputSlot = conn.outputSlot;
        def.connections.push_back(cd);
      }
    }
  }

  // 3. Register
  FILE *f = fopen("debug.txt", "a");
  if (f) {
    fprintf(f, "Registering new gate: %s with %zu nodes and %zu connections\n",
            def.name.c_str(), def.nodes.size(), def.connections.size());
    fclose(f);
  }

  // Store definition for serialization
  def.color =
      IM_COL32((int)(newGateColor[0] * 255), (int)(newGateColor[1] * 255),
               (int)(newGateColor[2] * 255), 200);

  customGateDefinitions.push_back(def);
  CustomGate::GateRegistry[def.name] = def;

  availableGates.push_back([def]() -> Gate * { return new CustomGate(def); });
}

void NodeEditor::Redraw() {
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
    if (ImGui::SmallButton("↻")) {
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

void NodeEditor::SaveGates(const std::string &filename) {
  FILE *f = fopen(filename.c_str(), "wb");
  if (!f)
    return;

  size_t count = customGateDefinitions.size();
  fwrite(&count, sizeof(size_t), 1, f);

  for (const auto &def : customGateDefinitions) {
    // Name
    size_t nameLen = def.name.size();
    fwrite(&nameLen, sizeof(size_t), 1, f);
    fwrite(def.name.c_str(), 1, nameLen, f);

    // Color
    fwrite(&def.color, sizeof(ImU32), 1, f);

    // Nodes
    size_t nodeCount = def.nodes.size();
    fwrite(&nodeCount, sizeof(size_t), 1, f);
    for (const auto &node : def.nodes) {
      size_t typeLen = node.type.size();
      fwrite(&typeLen, sizeof(size_t), 1, f);
      fwrite(node.type.c_str(), 1, typeLen, f);
      fwrite(&node.pos, sizeof(ImVec2), 1, f);
      fwrite(&node.id, sizeof(int), 1, f);
    }

    // Connections
    size_t connCount = def.connections.size();
    fwrite(&connCount, sizeof(size_t), 1, f);
    for (const auto &conn : def.connections) {
      fwrite(&conn.inputNodeId, sizeof(int), 1, f);

      size_t inSlotLen = conn.inputSlot.size();
      fwrite(&inSlotLen, sizeof(size_t), 1, f);
      fwrite(conn.inputSlot.c_str(), 1, inSlotLen, f);

      fwrite(&conn.outputNodeId, sizeof(int), 1, f);

      size_t outSlotLen = conn.outputSlot.size();
      fwrite(&outSlotLen, sizeof(size_t), 1, f);
      fwrite(conn.outputSlot.c_str(), 1, outSlotLen, f);
    }

    // Pin Indices
    size_t inPinCount = def.inputPinIndices.size();
    fwrite(&inPinCount, sizeof(size_t), 1, f);
    fwrite(def.inputPinIndices.data(), sizeof(int), inPinCount, f);

    size_t outPinCount = def.outputPinIndices.size();
    fwrite(&outPinCount, sizeof(size_t), 1, f);
    fwrite(def.outputPinIndices.data(), sizeof(int), outPinCount, f);
  }
  fclose(f);
}

void NodeEditor::LoadGates(const std::string &filename) {
  FILE *f = fopen(filename.c_str(), "rb");
  if (!f)
    return;

  customGateDefinitions.clear();

  size_t count = 0;
  fread(&count, sizeof(size_t), 1, f);

  for (size_t i = 0; i < count; i++) {
    GateDefinition def;

    // Name
    size_t nameLen = 0;
    fread(&nameLen, sizeof(size_t), 1, f);
    def.name.resize(nameLen);
    fread(&def.name[0], 1, nameLen, f);

    // Color
    fread(&def.color, sizeof(ImU32), 1, f);

    // Nodes
    size_t nodeCount = 0;
    fread(&nodeCount, sizeof(size_t), 1, f);
    for (size_t j = 0; j < nodeCount; j++) {
      NodeDefinition nd;
      size_t typeLen = 0;
      fread(&typeLen, sizeof(size_t), 1, f);
      nd.type.resize(typeLen);
      fread(&nd.type[0], 1, typeLen, f);
      fread(&nd.pos, sizeof(ImVec2), 1, f);
      fread(&nd.id, sizeof(int), 1, f);
      def.nodes.push_back(nd);
    }

    // Connections
    size_t connCount = 0;
    fread(&connCount, sizeof(size_t), 1, f);
    for (size_t j = 0; j < connCount; j++) {
      ConnectionDefinition cd;
      fread(&cd.inputNodeId, sizeof(int), 1, f);

      size_t inSlotLen = 0;
      fread(&inSlotLen, sizeof(size_t), 1, f);
      cd.inputSlot.resize(inSlotLen);
      fread(&cd.inputSlot[0], 1, inSlotLen, f);

      fread(&cd.outputNodeId, sizeof(int), 1, f);

      size_t outSlotLen = 0;
      fread(&outSlotLen, sizeof(size_t), 1, f);
      cd.outputSlot.resize(outSlotLen);
      fread(&cd.outputSlot[0], 1, outSlotLen, f);

      def.connections.push_back(cd);
    }

    // Pin Indices
    size_t inPinCount = 0;
    fread(&inPinCount, sizeof(size_t), 1, f);
    def.inputPinIndices.resize(inPinCount);
    fread(def.inputPinIndices.data(), sizeof(int), inPinCount, f);

    size_t outPinCount = 0;
    fread(&outPinCount, sizeof(size_t), 1, f);
    def.outputPinIndices.resize(outPinCount);
    fread(def.outputPinIndices.data(), sizeof(int), outPinCount, f);

    customGateDefinitions.push_back(def);
    CustomGate::GateRegistry[def.name] = def;
    availableGates.push_back([def]() -> Gate * { return new CustomGate(def); });
  }
  fclose(f);
}

NodeEditor::NodeEditor() {}
} // namespace Billyprints