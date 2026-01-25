#include "CustomGate.hpp"
#include "../Special/PinIn.hpp"
#include "../Special/PinOut.hpp"
#include "AND.hpp"
#include "NOT.hpp"

namespace Billyprints {

std::map<std::string, GateDefinition> CustomGate::GateRegistry;

Node *CreateNodeByType(const std::string &type) {
  if (type == "AND")
    return new AND();
  if (type == "NOT")
    return new NOT();
  if (type == "In")
    return new PinIn();
  if (type == "Out")
    return new PinOut();

  // Check Custom Gate Registry
  if (CustomGate::GateRegistry.count(type)) {
    return new CustomGate(CustomGate::GateRegistry[type]);
  }

  return nullptr;
}

CustomGate::CustomGate(const GateDefinition &def)
    : Gate(def.name.c_str(), {}, {}), definition(def) {
  title = _strdup(def.name.c_str()); // ImNodes needs a char*

  // 1. Create Internal Nodes
  std::map<int, Node *> nodeMap; // Map definition ID to actual Node*

  for (const auto &nodeDef : def.nodes) {
    Node *newNode = CreateNodeByType(nodeDef.type);
    if (newNode) {
      // newNode->pos = nodeDef.pos; // Position doesn't matter for logic, only
      // for editing if we allowed opening it
      internalNodes.push_back(newNode);
      nodeMap[nodeDef.id] = newNode;

      if (nodeDef.type == "In") {
        internalInputs.push_back((PinIn *)newNode);
      } else if (nodeDef.type == "Out") {
        internalOutputs.push_back((PinOut *)newNode);
      }
    }
  }

  // 2. Setup External Slots based on PinIn/PinOut counts
  // Sort internalInputs/Outputs based on some logic (e.g., Y position) if
  // needed, but for now relying on definition order (which should be sorted by
  // creation or position)

  inputSlotCount = (int)internalInputs.size();
  outputSlotCount = (int)internalOutputs.size();

  inputSlots.resize(inputSlotCount);
  outputSlots.resize(outputSlotCount);

  for (int i = 0; i < inputSlotCount; ++i) {
    char buf[16];
    if (inputSlotCount == 1)
      sprintf(buf, "in");
    else
      sprintf(buf, "in%d", i);
    inputSlots[i] = {strdup(buf), 1};
  }
  for (int i = 0; i < outputSlotCount; ++i) {
    char buf[16];
    if (outputSlotCount == 1)
      sprintf(buf, "out");
    else
      sprintf(buf, "out%d", i);
    outputSlots[i] = {strdup(buf), 1};
  }

  // 3. Create Internal Connections
  for (const auto &connDef : definition.connections) {
    if (nodeMap.count(connDef.inputNodeId) &&
        nodeMap.count(connDef.outputNodeId)) {
      Connection conn;
      conn.inputNode = nodeMap[connDef.inputNodeId];
      conn.inputSlot = connDef.inputSlot.c_str();
      conn.outputNode = nodeMap[connDef.outputNodeId];
      conn.outputSlot = connDef.outputSlot.c_str();

      // Link them virtually
      ((Node *)conn.inputNode)->connections.push_back(conn);
      ((Node *)conn.outputNode)->connections.push_back(conn);

      internalConnections.push_back(conn);
    }
  }
}

CustomGate::~CustomGate() {
  for (auto node : internalNodes) {
    delete node;
  }
}

bool CustomGate::Evaluate() {
  if (isEvaluating || lastEvaluatedFrame == Node::GlobalFrameCount) {
    return value;
  }

  isEvaluating = true;

  // Step A: Update Internal PinIns
  for (int i = 0; i < inputSlots.size(); ++i) {
    bool slotValue = false;
    char slotName[16];
    if (inputSlots.size() == 1)
      sprintf(slotName, "in");
    else
      sprintf(slotName, "in%d", i);

    for (const auto &conn : connections) {
      if (conn.inputNode == this && !conn.inputSlot.empty() &&
          strcmp(conn.inputSlot.c_str(), slotName) == 0) {
        Node *source = (Node *)conn.outputNode;
        slotValue = source->Evaluate();
        break;
      }
    }

    if (i < internalInputs.size()) {
      internalInputs[i]->value = slotValue;
    }
  }

  // Step B: Propagate Internal
  // Reduced passes + caching makes this much faster.
  // 3 passes is enough to settle most combinatorial logic without complex
  // feedback.
  for (int pass = 0; pass < 3; ++pass) {
    for (auto node : internalNodes) {
      node->Evaluate();
    }
  }

  // Step C: Set Output
  if (!internalOutputs.empty()) {
    value = internalOutputs[0]->value;
  } else {
    value = false;
  }

  lastEvaluatedFrame = Node::GlobalFrameCount;
  isEvaluating = false;
  return value;
}

} // namespace Billyprints
