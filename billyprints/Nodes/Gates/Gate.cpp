#include "Gate.hpp"

namespace Billyprints {
Gate::Gate(const char *_title, std::vector<ImNodes::Ez::SlotInfo> &&_inputSlots,
           std::vector<ImNodes::Ez::SlotInfo> &&_outputSlots)
    : Node(_title, std::move(_inputSlots), std::move(_outputSlots)) {}

ImU32 Gate::GetColor() const {
  std::string t = title;
  if (t == "AND")
    return IM_COL32(10, 30, 60, 255);
  if (t == "NOT")
    return IM_COL32(80, 20, 20, 255);
  if (t == "NAND")
    return IM_COL32(40, 10, 80, 255);
  return Node::GetColor();
}

void Gate::Render() {
  ImU32 color = GetColor();
  color = (color & 0x00FFFFFF) | 0xFF000000; // Force solid

  ImU32 borderColor =
      Evaluate() ? IM_COL32(255, 255, 255, 200) : IM_COL32(50, 50, 50, 50);

  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeTitleBarBg, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeTitleBarBgHovered, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeTitleBarBgActive, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeBodyBg, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeBodyBgHovered, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeBodyBgActive, color);
  ImNodes::Ez::PushStyleColor(ImNodesStyleCol_NodeBorder, borderColor);

  if (ImNodes::Ez::BeginNode(this, "", &pos, &selected)) {

    ImNodes::Ez::InputSlots(inputSlots.data(), inputSlotCount);

    ImGui::TextUnformatted(title);
    ImGui::SameLine();

    ImNodes::Ez::OutputSlots(outputSlots.data(), outputSlotCount);

    // Handle new connections
    void *inNode, *outNode;
    const char *inSlot, *outSlot;
    if (ImNodes::GetNewConnection(&inNode, &inSlot, &outNode, &outSlot)) {
      Connection new_connection;
      new_connection.inputNode = inNode;
      new_connection.inputSlot = inSlot;
      new_connection.outputNode = outNode;
      new_connection.outputSlot = outSlot;

      Node *inputNode = (Node *)new_connection.inputNode;
      Connection existingConnection = {};
      bool foundExisting = false;

      for (const auto &conn : inputNode->connections) {
        if (conn.inputNode == inputNode &&
            conn.inputSlot == new_connection.inputSlot) {
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

    // Render output connections
    for (const Connection &connection : connections) {
      if (connection.outputNode != this)
        continue;

      bool signal = Evaluate();
      ImColor activeColor = IM_COL32(255, 160, 20, 255);
      ImColor inactiveColor = IM_COL32(80, 90, 100, 255);

      auto *canvas = ImNodes::GetCurrentCanvas();
      ImColor originalConnectionColor = canvas->Colors[ImNodes::ColConnection];
      canvas->Colors[ImNodes::ColConnection] =
          signal ? activeColor : inactiveColor;

      if (!ImNodes::Connection(
              connection.inputNode, connection.inputSlot.c_str(),
              connection.outputNode, connection.outputSlot.c_str())) {
        ((Node *)connection.inputNode)->DeleteConnection(connection);
        ((Node *)connection.outputNode)->DeleteConnection(connection);
      }
      canvas->Colors[ImNodes::ColConnection] = originalConnectionColor;
    }

    ImNodes::Ez::EndNode();
    ImNodes::Ez::PopStyleColor(7);
  }
}
} // namespace Billyprints
