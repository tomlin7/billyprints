#pragma once

#include "Node.hpp"
#include "Gate.hpp"
#include "Source.hpp"
#include "AND.hpp"
#include "OR.hpp"
#include "NOT.hpp"
#include "Connection.hpp"

class NodeEditor {
    std::vector<Node*> nodes;
    std::vector<Node*(*)()> availableNodes {
        []() -> Node* {
            return new Source();
        },
        []() -> Node* {
            return new AND();
        },
        []() -> Node* {
            return new OR();
        },
        []() -> Node* {
            return new NOT();
        }
    };
    std::vector<Node* (*)()> available_gates{
        []() -> Node* {
            return new AND();
        },
        []() -> Node* {
            return new OR();
        },
        []() -> Node* {
            return new NOT();
        }
    };

    void RenderNode(Node* node);
    void RenderNodes();
    void RenderContextMenu();

public:
    NodeEditor();
    void Redraw();
};
