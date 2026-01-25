#include "PinIn.hpp"

namespace Billyprints {
PinIn::PinIn() : Node("In", {}, {{"out"}}) { value = true; };

bool PinIn::Evaluate() {
  if (isEvaluating || lastEvaluatedFrame == GlobalFrameCount)
    return value;
  lastEvaluatedFrame = GlobalFrameCount;
  return value;
};

void PinIn::Render() {
  ImU32 color = GetColor();
  color = (color & 0x00FFFFFF) | 0xFF000000;
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
    ImGui::SetWindowFontScale(0.8f);
    ImNodes::Ez::InputSlots(inputSlots.data(), inputSlotCount);

    ImGui::PushStyleColor(ImGuiCol_Button, value ? ImVec4(0, 0.6f, 0, 1)
                                                 : ImVec4(0.6f, 0, 0, 1));
    if (ImGui::Button(value ? "ON" : "OFF", ImVec2(40, 30))) {
      value = !value;
    }
    ImGui::PopStyleColor();

    ImNodes::Ez::OutputSlots(outputSlots.data(), outputSlotCount);

    // Logic for connections
    for (const Connection &connection : connections) {
      if (connection.outputNode != this)
        continue;
      bool signal = Evaluate();
      auto *canvas = ImNodes::GetCurrentCanvas();
      ImColor originalConnectionColor = canvas->Colors[ImNodes::ColConnection];
      canvas->Colors[ImNodes::ColConnection] =
          signal ? IM_COL32(255, 160, 20, 255) : IM_COL32(80, 90, 100, 255);
      if (!ImNodes::Connection(
              connection.inputNode, connection.inputSlot.c_str(),
              connection.outputNode, connection.outputSlot.c_str())) {
        ((Node *)connection.inputNode)->DeleteConnection(connection);
        ((Node *)connection.outputNode)->DeleteConnection(connection);
      }
      canvas->Colors[ImNodes::ColConnection] = originalConnectionColor;
    }

    ImGui::SetWindowFontScale(1.0f);
    ImNodes::Ez::EndNode();
    ImNodes::Ez::PopStyleColor(7);
  }
}
} // namespace Billyprints
