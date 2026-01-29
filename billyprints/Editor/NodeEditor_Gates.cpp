#include "../Nodes/Gates/CustomGate.hpp"
#include "NodeEditor.hpp"
#include <ImNodes.h>
#include <functional>
#include <imgui.h>
#include <map>
#include <string>
#include <vector>

namespace Billyprints {

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

void NodeEditor::SaveScene(const std::string &filename) {
  FILE *f = fopen(filename.c_str(), "wb");
  if (!f)
    return;

  // Build node ID map
  std::map<Node *, int> nodePtrToId;
  int idCounter = 0;

  // Write magic number for scene files
  const char magic[4] = {'B', 'P', 'S', '1'}; // Billyprints Scene v1
  fwrite(magic, 1, 4, f);

  // Write node count
  size_t nodeCount = nodes.size();
  fwrite(&nodeCount, sizeof(size_t), 1, f);

  // Write nodes
  for (auto *node : nodes) {
    nodePtrToId[node] = idCounter++;

    // Node type
    std::string type = node->title;
    size_t typeLen = type.size();
    fwrite(&typeLen, sizeof(size_t), 1, f);
    fwrite(type.c_str(), 1, typeLen, f);

    // Node position
    fwrite(&node->pos, sizeof(ImVec2), 1, f);
  }

  // Collect unique connections (only from output side to avoid duplicates)
  std::vector<ConnectionDefinition> connections;
  for (auto *node : nodes) {
    for (const auto &conn : node->connections) {
      if (conn.outputNode == node) {
        ConnectionDefinition cd;
        cd.inputNodeId = nodePtrToId[(Node *)conn.inputNode];
        cd.inputSlot = conn.inputSlot;
        cd.outputNodeId = nodePtrToId[(Node *)conn.outputNode];
        cd.outputSlot = conn.outputSlot;
        connections.push_back(cd);
      }
    }
  }

  // Write connection count
  size_t connCount = connections.size();
  fwrite(&connCount, sizeof(size_t), 1, f);

  // Write connections
  for (const auto &conn : connections) {
    fwrite(&conn.inputNodeId, sizeof(int), 1, f);

    size_t inSlotLen = conn.inputSlot.size();
    fwrite(&inSlotLen, sizeof(size_t), 1, f);
    fwrite(conn.inputSlot.c_str(), 1, inSlotLen, f);

    fwrite(&conn.outputNodeId, sizeof(int), 1, f);

    size_t outSlotLen = conn.outputSlot.size();
    fwrite(&outSlotLen, sizeof(size_t), 1, f);
    fwrite(conn.outputSlot.c_str(), 1, outSlotLen, f);
  }

  fclose(f);
}

void NodeEditor::LoadScene(const std::string &filename) {
  FILE *f = fopen(filename.c_str(), "rb");
  if (!f)
    return;

  // Verify magic number
  char magic[4];
  fread(magic, 1, 4, f);
  if (magic[0] != 'B' || magic[1] != 'P' || magic[2] != 'S' || magic[3] != '1') {
    fclose(f);
    return; // Invalid file format
  }

  // Clear existing nodes
  for (auto *node : nodes) {
    delete node;
  }
  nodes.clear();

  // Read node count
  size_t nodeCount = 0;
  fread(&nodeCount, sizeof(size_t), 1, f);

  // Map for ID to node pointer
  std::map<int, Node *> idToNode;

  // Read nodes
  for (size_t i = 0; i < nodeCount; i++) {
    // Node type
    size_t typeLen = 0;
    fread(&typeLen, sizeof(size_t), 1, f);
    std::string type;
    type.resize(typeLen);
    fread(&type[0], 1, typeLen, f);

    // Node position
    ImVec2 pos;
    fread(&pos, sizeof(ImVec2), 1, f);

    // Create node
    Node *node = CreateNodeByType(type);
    if (node) {
      node->pos = pos;
      node->id = "n" + std::to_string(i);
      nodes.push_back(node);
      idToNode[(int)i] = node;
    }
  }

  // Read connection count
  size_t connCount = 0;
  fread(&connCount, sizeof(size_t), 1, f);

  // Read connections
  for (size_t i = 0; i < connCount; i++) {
    int inputNodeId;
    fread(&inputNodeId, sizeof(int), 1, f);

    size_t inSlotLen = 0;
    fread(&inSlotLen, sizeof(size_t), 1, f);
    std::string inputSlot;
    inputSlot.resize(inSlotLen);
    fread(&inputSlot[0], 1, inSlotLen, f);

    int outputNodeId;
    fread(&outputNodeId, sizeof(int), 1, f);

    size_t outSlotLen = 0;
    fread(&outSlotLen, sizeof(size_t), 1, f);
    std::string outputSlot;
    outputSlot.resize(outSlotLen);
    fread(&outputSlot[0], 1, outSlotLen, f);

    // Create connection if both nodes exist
    if (idToNode.count(inputNodeId) && idToNode.count(outputNodeId)) {
      Connection conn;
      conn.inputNode = idToNode[inputNodeId];
      conn.inputSlot = inputSlot;
      conn.outputNode = idToNode[outputNodeId];
      conn.outputSlot = outputSlot;

      ((Node *)conn.inputNode)->connections.push_back(conn);
      ((Node *)conn.outputNode)->connections.push_back(conn);
    }
  }

  fclose(f);

  // Update script from loaded nodes
  UpdateScriptFromNodes();
  lastParsedScript = currentScript;
}

} // namespace Billyprints
