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
  ImU32 titleColor = IM_COL32(50, 50, 50, 255);
  std::string t = node->title;
  if (t == "AND")
    titleColor = IM_COL32(0, 100, 200, 255);
  else if (t == "NOT")
    titleColor = IM_COL32(200, 50, 50, 255);
  else if (t == "NAND")
    titleColor = IM_COL32(120, 0, 220, 255);
  else if (t == "Input" || t == "PinIn")
    titleColor = IM_COL32(0, 150, 0, 255);

  auto *canvas = ImNodes::GetCurrentCanvas();
  ImColor originalTitleColor = canvas->Colors[ImNodes::ColNodeBg];
  ImColor originalTitleColorSelected = canvas->Colors[ImNodes::ColNodeActiveBg];

  canvas->Colors[ImNodes::ColNodeBg] = titleColor;
  canvas->Colors[ImNodes::ColNodeActiveBg] =
      titleColor; // Or slightly brighter? Using same for now

  if (ImNodes::Ez::BeginNode(node, node->title, &node->pos, &node->selected)) {
    ImNodes::Ez::InputSlots(node->inputSlots.data(),
                            static_cast<int>(node->inputSlots.size()));
    ImGui::Text("%d", (Node *)node->Evaluate());
    ImNodes::Ez::OutputSlots(node->outputSlots.data(),
                             static_cast<int>(node->outputSlots.size()));

    Connection new_connection;
    if (ImNodes::GetNewConnection(
            &new_connection.inputNode, &new_connection.inputSlot,
            &new_connection.outputNode, &new_connection.outputSlot)) {
      ((Node *)new_connection.inputNode)->connections.push_back(new_connection);
      ((Node *)new_connection.outputNode)
          ->connections.push_back(new_connection);
    }

    // Render connections
    for (const Connection &connection : node->connections) {
      if (connection.outputNode != node)
        continue;

      bool signal = ((Node *)connection.outputNode)->Evaluate();
      ImColor activeColor = IM_COL32(255, 50, 50, 255);
      ImColor inactiveColor = IM_COL32(100, 100, 100, 255);

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
    if (titleStr == "Input" || titleStr == "PinIn")
      def.inputPinIndices.push_back(nd.id);
    else if (titleStr == "Output" || titleStr == "PinOut")
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
  availableGates.push_back([def]() -> Gate * { return new CustomGate(def); });
}

void NodeEditor::Redraw() {
  auto context = ImNodes::Ez::CreateContext();
  IM_UNUSED(context);

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
          ImGui::OpenPopup("CreateGatePopup");
        }
        if (ImGui::MenuItem("Close", "Ctrl+W")) {
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopupModal("CreateGatePopup", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::InputText("Gate Name", gateName, 128);
      if (ImGui::Button("Create", ImVec2(120, 0))) {
        CreateGate();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImNodes::Ez::BeginCanvas();
    ImNodes::Ez::PushStyleVar(ImNodesStyleVar_NodeRounding, 5.0f);

    RenderNodes();

    RenderContextMenu();

    ImNodes::Ez::EndCanvas();
    ImNodes::Ez::PopStyleVar();
    ImGui::End();
  }
}

NodeEditor::NodeEditor() {}
} // namespace Billyprints