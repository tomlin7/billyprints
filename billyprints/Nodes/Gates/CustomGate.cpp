#include "CustomGate.hpp"
#include "../Special/PinIn.hpp"
#include "../Special/PinOut.hpp"
#include "AND.hpp"
#include "NOT.hpp"
#include "OR.hpp" // Just in case, though we removed it from menu, the class exists

namespace Billyprints {

Node *CreateNodeByType(const std::string &type) {
  if (type == "AND")
    return new AND();
  if (type == "NOT")
    return new NOT();
  if (type == "PinIn" || type == "Pin In")
    return new PinIn();
  if (type == "PinOut" || type == "Pin Out")
    return new PinOut();
  return nullptr; // Custom gates inside custom gates not supported yet for
                  // simplicity
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

      if (nodeDef.type == "PinIn" || nodeDef.type == "Pin In") {
        internalInputs.push_back((PinIn *)newNode);
      } else if (nodeDef.type == "PinOut" || nodeDef.type == "Pin Out") {
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
    sprintf(buf, "In %d", i);
    inputSlots[i] = {strdup(buf), 1};
  }
  for (int i = 0; i < outputSlotCount; ++i) {
    char buf[16];
    sprintf(buf, "Out %d", i);
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
  // 1. Copy External Input Slots to Internal PinIn Nodes
  // The slot value is stored in... actually ImNodes::Ez doesn't store values in
  // slots. We need to look at who is connected to US. Wait, the standard
  // Node::Evaluate() usually returns the 'value'. But for multi-input/output,
  // the logic is different. The standard Gate::Evaluate implementation (if any)
  // usually reads dependencies.

  // Let's assume the callers (downstream nodes) call Evaluate() on us.
  // Or we are called by the engine.

  // Standard approach in this codebase seems to be pull-based:
  // A node evaluates itself by checking its inputs.

  // Step A: Update Internal PinIns
  // We need to know specific input values.
  // Since `Evaluate` returns a bool, it implies single output?
  // Existing gates (AND) have "Result" slot.
  // If CustomGate has multiple outputs, `Evaluate()` is ambiguous.
  // But the task implies "Gate" -> usually single output logic block, OR we
  // support multi-output.

  // Current Node.hpp: virtual bool Evaluate();
  // This suggests single output being the return value.
  // But complex circuits might have multiple outputs.
  // For now, let's update internal state.

  // Issue: How do we get the input values from the *external* connections?
  // The `Node` class has `connections`.
  // We need to find which connection feeds into input slot i.

  for (int i = 0; i < inputSlots.size(); ++i) {
    bool slotValue = false;
    char slotName[16];
    sprintf(slotName, "In %d", i);

    // Find connection to this slot
    for (const auto &conn : connections) {
      if (conn.inputNode == this && conn.inputSlot &&
          strcmp(conn.inputSlot, slotName) == 0) {
        // Found a feeder. Evaluate it.
        Node *source = (Node *)conn.outputNode;
        slotValue = source->Evaluate(); // Simple pull.
        break;
      }
    }

    // Push to internal Pinin
    if (i < internalInputs.size()) {
      internalInputs[i]->value = slotValue;
    }
  }

  // Step B: Propagate Internal
  // Simple multiple passes to settle the circuit (inefficient but works for
  // small combinatorial)
  int maxPasses = internalNodes.size() + 2;
  for (int pass = 0; pass < maxPasses; ++pass) {
    for (auto node : internalNodes) {
      // PinIn doesn't need eval, it's set.
      // PinOut just reads input.
      // Standard gates read inputs and compute.
      node->Evaluate();
    }
  }

  // Step C: Set Output
  // If we have multiple outputs, this single return `bool` is insufficient.
  // However, if we only use the first PinOut for the return value...
  if (!internalOutputs.empty()) {
    return internalOutputs[0]->value;
  }
  return false;
}

} // namespace Billyprints
