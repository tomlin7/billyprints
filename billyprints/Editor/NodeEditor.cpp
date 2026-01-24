#include "NodeEditor.hpp"
#include "../Nodes/Gates/CustomGate.hpp"
#include <functional>
#include <imgui.h>
#include <imnodes.h>
#include <iostream>
#include <map>
#include <set>

namespace Billyprints {
inline void NodeEditor::RenderNode(Node *node) {
  // Glassy Nodes (Semi-transparent)
  ImU32 titleColor = IM_COL32(30, 30, 35, 180);
  std::string t = node->title;
  if (t == "AND")
    titleColor = IM_COL32(0, 50, 100, 180); // Dark Blue Glass
  else if (t == "NOT")
    titleColor = IM_COL32(100, 25, 25, 180); // Dark Red Glass
  else if (t == "NAND")
    titleColor = IM_COL32(60, 0, 110, 180); // Dark Purple Glass
  else if (t == "Pin In" || t == "PinIn")
    titleColor = IM_COL32(0, 80, 0, 180); // Dark Green Glass

  auto *canvas = ImNodes::GetCurrentCanvas();
  ImColor originalTitleColor = canvas->Colors[ImNodes::ColNodeBg];
  ImColor originalTitleColorSelected = canvas->Colors[ImNodes::ColNodeActiveBg];

  canvas->Colors[ImNodes::ColNodeBg] = titleColor;
  canvas->Colors[ImNodes::ColNodeActiveBg] =
      IM_COL32((titleColor >> 0) & 0xFF, (titleColor >> 8) & 0xFF,
               (titleColor >> 16) & 0xFF,
               230); // Brighter alpha when selected

  if (ImNodes::Ez::BeginNode(node, node->title, &node->pos, &node->selected)) {
    // ... input slots ...

    ImNodes::Ez::InputSlots(node->inputSlots.data(),
                            static_cast<int>(node->inputSlots.size()));

    std::string titleStr = node->title;
    if (titleStr == "Pin In" || titleStr == "PinIn") {
      ImGui::PushStyleColor(ImGuiCol_Button, node->value
                                                 ? ImVec4(0, 0.6f, 0, 1)
                                                 : ImVec4(0.6f, 0, 0, 1));
      if (ImGui::Button(node->value ? "ON" : "OFF", ImVec2(40, 30))) {
        node->value = !node->value;
      }
      ImGui::PopStyleColor();
    } else if (titleStr == "Pin Out" || titleStr == "PinOut") {
      bool signal = node->Evaluate();
      ImGui::PushStyleColor(ImGuiCol_Button, signal
                                                 ? ImVec4(0, 0.8f, 0, 1)
                                                 : ImVec4(0.1f, 0.1f, 0.1f, 1));
      ImGui::Button(signal ? "HIGH" : "LOW", ImVec2(40, 30));
      ImGui::PopStyleColor();
    } else {
      ImGui::Text("%d", (Node *)node->Evaluate());
    }
    ImNodes::Ez::OutputSlots(node->outputSlots.data(),
                             static_cast<int>(node->outputSlots.size()));

    Connection new_connection;
    if (ImNodes::GetNewConnection(
            &new_connection.inputNode, &new_connection.inputSlot,
            &new_connection.outputNode, &new_connection.outputSlot)) {

      // Enforce Single Input Connection Rule:
      // Check if input node already has a connection to this specific input
      // slot
      Node *inputNode = (Node *)new_connection.inputNode;
      Connection existingConnection = {};
      bool foundExisting = false;

      for (const auto &conn : inputNode->connections) {
        if (conn.inputNode == inputNode &&
            std::string(conn.inputSlot) ==
                std::string(new_connection.inputSlot)) {
          existingConnection = conn;
          foundExisting = true;
          break;
        }
      }

      if (foundExisting) {
        ((Node *)existingConnection.outputNode)
            ->DeleteConnection(existingConnection);
        inputNode->DeleteConnection(existingConnection);
      }

      ((Node *)new_connection.inputNode)->connections.push_back(new_connection);
      ((Node *)new_connection.outputNode)
          ->connections.push_back(new_connection);
    }

    // Render connections
    for (const Connection &connection : node->connections) {
      if (connection.outputNode != node)
        continue;

      bool signal = ((Node *)connection.outputNode)->Evaluate();
      // Neon Styles
      ImColor activeColor = IM_COL32(255, 160, 20, 255);  // Neon Orange
      ImColor inactiveColor = IM_COL32(80, 90, 100, 255); // Dim Blue-Gray

      auto *canvas = ImNodes::GetCurrentCanvas();
      ImColor originalConnectionColor = canvas->Colors[ImNodes::ColConnection];
      canvas->Colors[ImNodes::ColConnection] =
          signal ? activeColor : inactiveColor;

      if (!ImNodes::Connection(connection.inputNode, connection.inputSlot,
                               connection.outputNode, connection.outputSlot)) {
        ((Node *)connection.inputNode)->DeleteConnection(connection);
        ((Node *)connection.outputNode)->DeleteConnection(connection);
      }
      canvas->Colors[ImNodes::ColConnection] = originalConnectionColor;
    }
    ImNodes::Ez::EndNode();

    // Restore node colors
    canvas->Colors[ImNodes::ColNodeBg] = originalTitleColor;
    canvas->Colors[ImNodes::ColNodeActiveBg] = originalTitleColorSelected;
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
    if (titleStr == "Input" || titleStr == "Pin In")
      def.inputPinIndices.push_back(nd.id);
    else if (titleStr == "Output" || titleStr == "Pin Out")
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
  customGateDefinitions.push_back(def);

  availableGates.push_back([def]() -> Gate * { return new CustomGate(def); });
}

void NodeEditor::Redraw() {
  auto context = ImNodes::Ez::CreateContext();
  IM_UNUSED(context);

  // Main Menu Bar
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save Custom Gates...")) {
        openSaveGatePopup = true;
      }
      if (ImGui::MenuItem("Load Custom Gates...")) {
        openLoadGatePopup = true;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) {
        exit(0);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  // Save Gate Popup
  if (openSaveGatePopup) {
    ImGui::OpenPopup("SaveGatePopup");
    openSaveGatePopup = false;
  }
  if (ImGui::BeginPopupModal("SaveGatePopup", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Filename", currentFilename, 128);
    if (ImGui::Button("Save", ImVec2(120, 0))) {
      SaveGates(std::string(currentFilename));
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
    if (ImGui::Button("Load", ImVec2(120, 0))) {
      LoadGates(std::string(currentFilename));
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

  if (ImGui::Begin("Billyprints", NULL, window_flags)) {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open..", "Ctrl+O")) {
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
        }
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
      ImGui::EndMainMenuBar();
    }

    if (openCreateGatePopup) {
      ImGui::OpenPopup("CreateGatePopup");
      openCreateGatePopup = false;
    }

    if (ImGui::BeginPopupModal("CreateGatePopup", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::InputText("Gate Name", gateName, 128);
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

  ImNodes::Ez::BeginCanvas();

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
  ImNodes::Ez::PopStyleVar(1);
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
    availableGates.push_back([def]() -> Gate * { return new CustomGate(def); });
  }
  fclose(f);
}

NodeEditor::NodeEditor() {}
} // namespace Billyprints