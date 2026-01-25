#include "../Nodes/Gates/CustomGate.hpp"
#include "../Nodes/Special/PinIn.hpp"
#include "NodeEditor.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Billyprints {

void NodeEditor::UpdateScriptFromNodes() {
  EditorTab *tab = GetActiveTab();
  if (!tab)
    return;

  std::stringstream ss;
  std::map<Node *, std::string> nodeToId;

  // First pass: Assign IDs
  for (int i = 0; i < tab->nodes.size(); ++i) {
    if (tab->nodes[i]->id.empty()) {
      // Find a unique ID
      while (true) {
        std::string candidate = "n" + std::to_string(tab->autoIdCounter++);
        bool clash = false;
        for (const auto &n : tab->nodes) {
          if (n->id == candidate) {
            clash = true;
            break;
          }
        }
        if (!clash) {
          tab->nodes[i]->id = candidate;
          break;
        }
      }
    }
    nodeToId[tab->nodes[i]] = tab->nodes[i]->id;
  }

  for (int i = 0; i < tab->nodes.size(); ++i) {
    std::string type = tab->nodes[i]->title;
    ss << type << " " << tab->nodes[i]->id << " @ " << (int)tab->nodes[i]->pos.x
       << ", " << (int)tab->nodes[i]->pos.y;

    if (type == "In") {
      PinIn *pin = (PinIn *)tab->nodes[i];
      if (pin->isMomentary)
        ss << " momentary";
    }

    ss << "\n";
  }
  ss << "\n";
  for (auto *node : tab->nodes) {
    for (const auto &conn : node->connections) {
      if (conn.outputNode == node) {
        ss << nodeToId[(Node *)conn.outputNode] << "." << conn.outputSlot
           << " -> " << nodeToId[(Node *)conn.inputNode] << "."
           << conn.inputSlot << "\n";
      }
    }
  }
  tab->currentScript = ss.str();
}

void NodeEditor::UpdateNodesFromScript() {
  EditorTab *tab = GetActiveTab();
  if (!tab)
    return;

  if (tab->currentScript == tab->lastParsedScript)
    return;
  tab->lastParsedScript = tab->currentScript;
  tab->scriptError = "";

  auto trim = [](std::string &s) {
    if (s.empty())
      return;
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    size_t last = s.find_last_not_of(" \t\n\r");
    if (last != std::string::npos)
      s.erase(last + 1);
  };

  for (auto *n : tab->nodes)
    delete n;
  tab->nodes.clear();

  std::stringstream ss(tab->currentScript);
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
          tab->scriptError +=
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
          tab->nodes.push_back(n);
          idToNode[id] = n;
        } else {
          tab->scriptError += "Line " + std::to_string(lineNum) +
                              ": Unknown type " + type + "\n";
        }
      }
    } catch (...) {
      tab->scriptError +=
          "Line " + std::to_string(lineNum) + ": Unexpected error\n";
    }
  }
}
} // namespace Billyprints
