#include "../Nodes/Gates/CustomGate.hpp"
#include "../Nodes/Special/PinIn.hpp"
#include "NodeEditor.hpp"
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace Billyprints {

// Helper to trim whitespace
static void trimStr(std::string &s) {
  if (s.empty())
    return;
  s.erase(0, s.find_first_not_of(" \t\n\r"));
  size_t last = s.find_last_not_of(" \t\n\r");
  if (last != std::string::npos)
    s.erase(last + 1);
}

// Helper to split string by delimiter
static std::vector<std::string> splitStr(const std::string &s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    trimStr(item);
    if (!item.empty())
      result.push_back(item);
  }
  return result;
}

// Parse and register a custom gate definition from script
// Syntax: define Name(in1, in2) -> (out1, out2):
//           out1 = in1 OP in2
//         end
static bool ParseGateDefinition(const std::string &defBlock,
                                std::string &errorOut) {
  std::stringstream ss(defBlock);
  std::string line;
  std::string gateName;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  std::vector<std::pair<std::string, std::string>> assignments; // output = expr

  // Parse first line: define Name(in1, in2) -> (out1, out2):
  if (!std::getline(ss, line)) {
    errorOut = "Empty define block";
    return false;
  }
  trimStr(line);

  // Remove "define " prefix
  if (line.substr(0, 7) != "define ") {
    errorOut = "Block must start with 'define'";
    return false;
  }
  line = line.substr(7);
  trimStr(line);

  // Extract gate name
  size_t parenPos = line.find('(');
  if (parenPos == std::string::npos) {
    errorOut = "Missing '(' in define";
    return false;
  }
  gateName = line.substr(0, parenPos);
  trimStr(gateName);

  // Extract inputs: between ( and )
  size_t closeParenPos = line.find(')');
  if (closeParenPos == std::string::npos || closeParenPos <= parenPos) {
    errorOut = "Missing ')' for inputs";
    return false;
  }
  std::string inputsStr = line.substr(parenPos + 1, closeParenPos - parenPos - 1);
  inputs = splitStr(inputsStr, ',');

  // Find -> and outputs
  size_t arrowPos = line.find("->");
  if (arrowPos == std::string::npos) {
    errorOut = "Missing '->' in define";
    return false;
  }

  std::string afterArrow = line.substr(arrowPos + 2);
  trimStr(afterArrow);

  // Extract outputs: between ( and ):
  size_t outOpenParen = afterArrow.find('(');
  size_t outCloseParen = afterArrow.find(')');
  if (outOpenParen == std::string::npos || outCloseParen == std::string::npos) {
    errorOut = "Missing output parentheses";
    return false;
  }
  std::string outputsStr =
      afterArrow.substr(outOpenParen + 1, outCloseParen - outOpenParen - 1);
  outputs = splitStr(outputsStr, ',');

  if (gateName.empty() || inputs.empty() || outputs.empty()) {
    errorOut = "Gate must have name, inputs, and outputs";
    return false;
  }

  // Parse body: assignments like "out = in1 OP in2" or "out = NOT in1"
  while (std::getline(ss, line)) {
    trimStr(line);
    if (line.empty() || line == "end")
      continue;
    if (line[0] == '/' && line.size() > 1 && line[1] == '/')
      continue;

    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) {
      errorOut = "Invalid assignment: " + line;
      return false;
    }
    std::string lhs = line.substr(0, eqPos);
    std::string rhs = line.substr(eqPos + 1);
    trimStr(lhs);
    trimStr(rhs);
    assignments.push_back({lhs, rhs});
  }

  // Now build the GateDefinition
  GateDefinition def;
  def.name = gateName;
  def.color = IM_COL32(60, 80, 120, 200); // Default blue-ish color

  std::map<std::string, int> signalToNodeId; // Maps signal name to node ID
  int nodeIdCounter = 0;
  float yPos = 0;

  // Create PinIn nodes for each input
  for (const auto &inputName : inputs) {
    NodeDefinition nd;
    nd.type = "In";
    nd.id = nodeIdCounter;
    nd.pos = ImVec2(0, yPos);
    yPos += 60;
    def.nodes.push_back(nd);
    def.inputPinIndices.push_back(nodeIdCounter);
    signalToNodeId[inputName] = nodeIdCounter;
    nodeIdCounter++;
  }

  // Helper lambdas for creating nodes and connections
  auto createNode = [&](const std::string &type, float x, float y) -> int {
    NodeDefinition nd;
    nd.type = type;
    nd.id = nodeIdCounter;
    nd.pos = ImVec2(x, y);
    def.nodes.push_back(nd);
    return nodeIdCounter++;
  };

  auto connect = [&](int fromNode, const std::string &fromSlot, int toNode,
                     const std::string &toSlot) {
    ConnectionDefinition cd;
    cd.outputNodeId = fromNode;
    cd.outputSlot = fromSlot;
    cd.inputNodeId = toNode;
    cd.inputSlot = toSlot;
    def.connections.push_back(cd);
  };

  // Process each assignment to create gate nodes
  float gateX = 150;
  float gateY = 0;

  for (const auto &[outSignal, expr] : assignments) {
    // Parse expression: "in1 OP in2", "NOT in1", or "CustomGate(in1, in2)"
    std::string gateType;
    std::string operand1, operand2;
    std::vector<std::string> callArgs;

    // Check for NOT (unary)
    if (expr.substr(0, 4) == "NOT ") {
      gateType = "NOT";
      operand1 = expr.substr(4);
      trimStr(operand1);
    }
    // Check for AND (binary)
    else if (expr.find(" AND ") != std::string::npos) {
      size_t opPos = expr.find(" AND ");
      gateType = "AND";
      operand1 = expr.substr(0, opPos);
      operand2 = expr.substr(opPos + 5);
      trimStr(operand1);
      trimStr(operand2);
    }
    // Check for custom gate call: GateName(arg1, arg2, ...)
    else if (expr.find('(') != std::string::npos) {
      size_t parenPos = expr.find('(');
      size_t closePos = expr.rfind(')'); // Use rfind to get the LAST closing paren
      if (closePos != std::string::npos && closePos > parenPos) {
        gateType = expr.substr(0, parenPos);
        trimStr(gateType);
        std::string argsStr = expr.substr(parenPos + 1, closePos - parenPos - 1);
        // Note: nested calls like NAND(NAND(a,a), NAND(b,b)) are NOT supported
        // Use intermediate signals instead
        callArgs = splitStr(argsStr, ',');
      }
    }
    // Check for passthrough (out = in)
    else if (!expr.empty() && signalToNodeId.count(expr)) {
      signalToNodeId[outSignal] = signalToNodeId[expr];
      continue;
    } else {
      errorOut = "Unknown expression: " + expr;
      return false;
    }

    int resultNodeId = -1;

    if (gateType == "NOT") {
      if (!signalToNodeId.count(operand1)) {
        errorOut = "Unknown signal: " + operand1;
        return false;
      }
      int notGate = createNode("NOT", gateX, gateY);
      gateY += 50;
      connect(signalToNodeId[operand1], "out", notGate, "in");
      resultNodeId = notGate;

    } else if (gateType == "AND") {
      if (!signalToNodeId.count(operand1)) {
        errorOut = "Unknown signal: " + operand1;
        return false;
      }
      if (!signalToNodeId.count(operand2)) {
        errorOut = "Unknown signal: " + operand2;
        return false;
      }
      int andGate = createNode("AND", gateX, gateY);
      gateY += 50;
      connect(signalToNodeId[operand1], "out", andGate, "in0");
      connect(signalToNodeId[operand2], "out", andGate, "in1");
      resultNodeId = andGate;

    } else if (!callArgs.empty()) {
      // Custom gate call - check if gate exists in registry
      if (!CustomGate::GateRegistry.count(gateType)) {
        errorOut = "Unknown gate type: " + gateType +
                   " (make sure to load the gate library first, or define it earlier in the script)";
        return false;
      }

      // Validate arguments - no nested calls allowed
      for (const auto &arg : callArgs) {
        if (arg.find('(') != std::string::npos || arg.find(')') != std::string::npos) {
          errorOut = "Nested gate calls not supported. Use intermediate signals instead. "
                     "Example: t1 = NAND(a, a) then out = NAND(t1, t2)";
          return false;
        }
      }

      // Create the custom gate node
      int customGate = createNode(gateType, gateX, gateY);
      gateY += 60;

      // Connect arguments to inputs
      const auto &gateDef = CustomGate::GateRegistry[gateType];
      for (size_t i = 0; i < callArgs.size() && i < gateDef.inputPinIndices.size(); ++i) {
        if (!signalToNodeId.count(callArgs[i])) {
          errorOut = "Unknown signal: " + callArgs[i] + " in call to " + gateType;
          return false;
        }
        std::string inSlot = (gateDef.inputPinIndices.size() == 1) ? "in" : "in" + std::to_string(i);
        connect(signalToNodeId[callArgs[i]], "out", customGate, inSlot);
      }
      resultNodeId = customGate;

    } else {
      errorOut = "Invalid expression: " + expr;
      return false;
    }

    signalToNodeId[outSignal] = resultNodeId;
  }

  // Create PinOut nodes for each output
  float outX = 300;
  float outY = 0;
  for (const auto &outputName : outputs) {
    NodeDefinition nd;
    nd.type = "Out";
    nd.id = nodeIdCounter;
    nd.pos = ImVec2(outX, outY);
    outY += 60;
    def.nodes.push_back(nd);
    def.outputPinIndices.push_back(nodeIdCounter);

    // Connect the signal to this output
    if (signalToNodeId.count(outputName)) {
      ConnectionDefinition cd;
      cd.outputNodeId = signalToNodeId[outputName];
      cd.outputSlot = "out";
      cd.inputNodeId = nodeIdCounter;
      cd.inputSlot = "in";
      def.connections.push_back(cd);
    } else {
      errorOut = "Output signal not defined: " + outputName;
      return false;
    }
    nodeIdCounter++;
  }

  // Register the gate
  CustomGate::GateRegistry[def.name] = def;

  return true;
}

// Extract all define...end blocks from script and parse them
// Returns the define blocks in 'definitions' for preservation
static std::string ExtractAndParseDefinitions(const std::string &script,
                                              std::string &remaining,
                                              std::string &definitions,
                                              std::string &errorOut) {
  remaining = "";
  definitions = "";
  std::stringstream ss(script);
  std::string line;
  bool inDefine = false;
  std::string currentDefine;
  std::string outsideDefine;
  std::string allDefinitions;

  while (std::getline(ss, line)) {
    std::string trimmed = line;
    trimStr(trimmed);

    if (!inDefine && trimmed.substr(0, 7) == "define ") {
      inDefine = true;
      currentDefine = line + "\n";
    } else if (inDefine) {
      currentDefine += line + "\n";
      if (trimmed == "end") {
        // Parse this definition
        std::string err;
        if (!ParseGateDefinition(currentDefine, err)) {
          errorOut += "Define error: " + err + "\n";
        } else {
          // Successfully parsed, preserve the block
          allDefinitions += currentDefine + "\n";
        }
        inDefine = false;
        currentDefine = "";
      }
    } else {
      outsideDefine += line + "\n";
    }
  }

  if (inDefine) {
    errorOut += "Unclosed define block\n";
  }

  remaining = outsideDefine;
  definitions = allDefinitions;
  return errorOut;
}

void NodeEditor::UpdateScriptFromNodes() {
  std::stringstream ss;
  std::map<Node *, std::string> nodeToId;
  int autoIdCounter = 0;

  // Include any preserved gate definitions at the top
  if (!scriptDefinitions.empty()) {
    ss << scriptDefinitions;
  }

  // First pass: Assign IDs
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i]->id.empty()) {
      // Find a unique ID
      while (true) {
        std::string candidate = "n" + std::to_string(autoIdCounter++);
        bool clash = false;
        for (const auto &n : nodes) {
          if (n->id == candidate) {
            clash = true;
            break;
          }
        }
        if (!clash) {
          nodes[i]->id = candidate;
          break;
        }
      }
    }
    nodeToId[nodes[i]] = nodes[i]->id;
  }

  for (int i = 0; i < nodes.size(); ++i) {
    std::string type = nodes[i]->title;
    ss << type << " " << nodes[i]->id << " @ " << (int)nodes[i]->pos.x << ", "
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
        ss << nodeToId[(Node *)conn.outputNode] << "." << conn.outputSlot
           << " -> " << nodeToId[(Node *)conn.inputNode] << "."
           << conn.inputSlot << "\n";
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

  // First pass: Extract and parse custom gate definitions
  std::string remainingScript;
  std::string defErrors;
  ExtractAndParseDefinitions(currentScript, remainingScript, scriptDefinitions,
                             defErrors);
  if (!defErrors.empty()) {
    scriptError += defErrors;
  }

  // Second pass: Parse nodes and connections from remaining script
  std::stringstream ss(remainingScript);
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
          n->id = id;
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
} // namespace Billyprints
