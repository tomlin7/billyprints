#pragma once

#include "Node.hpp"
#include "Gate.hpp"
#include "Gates.hpp"
#include "Nodes.hpp"
#include "Connection.hpp"

namespace Billyprints {
    class NodeEditor {
        std::vector<Node*> nodes;

        void RenderNode(Node* node);
        void RenderNodes();
        void RenderContextMenu();

    public:
        NodeEditor();
        void Redraw();
    };
}