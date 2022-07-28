#include "NodeEditor.hpp"
#include <iostream>

namespace Billyprints {
    inline void NodeEditor::RenderNode(Node* node) {
        if (ImNodes::Ez::BeginNode(node, node->title, &node->pos, &node->selected))
        {
            ImNodes::Ez::InputSlots(node->inputSlots.data(), static_cast<int>(node->inputSlots.size()));
            ImGui::Text("%d", (Node*)node->Evaluate());
            ImNodes::Ez::OutputSlots(node->outputSlots.data(), static_cast<int>(node->outputSlots.size()));

            Connection new_connection;
            if (ImNodes::GetNewConnection(&new_connection.inputNode, &new_connection.inputSlot,
                &new_connection.outputNode, &new_connection.outputSlot))
            {
                ((Node*)new_connection.inputNode)->connections.push_back(new_connection);
                ((Node*)new_connection.outputNode)->connections.push_back(new_connection);
            }

            // Render connections
            for (const Connection& connection : node->connections)
            {
                if (connection.outputNode != node)
                    continue;

                if (!ImNodes::Connection(connection.inputNode, connection.inputSlot, connection.outputNode,
                    connection.outputSlot))
                {
                    ((Node*)connection.inputNode)->DeleteConnection(connection);
                    ((Node*)connection.outputNode)->DeleteConnection(connection);
                }
            }
            ImNodes::Ez::EndNode();
        }
    }

    inline void NodeEditor::RenderNodes() {
        for (auto it = nodes.begin(); it != nodes.end();)
        {
            Node* node = *it;

            RenderNode(node);

            if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete) && ImGui::IsWindowFocused())
            {
                for (auto& connection : node->connections)
                {
                    if (connection.outputNode == node)
                    {
                        ((Node*)connection.inputNode)->DeleteConnection(connection);
                    }
                    else
                    {
                        ((Node*)connection.outputNode)->DeleteConnection(connection);
                    }
                }
                node->connections.clear();

                delete node;
                it = nodes.erase(it);
            }
            else
                ++it;
        }
    }

    inline void NodeEditor::RenderContextMenu() {
        if (ImGui::BeginPopupContextWindow("NodesContextMenu")) {
            for (const auto& desc : availableNodes)
            {
                auto item = desc();
                if (ImGui::MenuItem(item->title))
                {
                    nodes.push_back(item);
                    ImNodes::AutoPositionNode(nodes.back());
                }
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Gates")) {
                for (const auto& desc : availableGates)
                {
                    auto item = desc();
                    if (ImGui::MenuItem(item->title))
                    {
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

    void NodeEditor::Redraw() {
        auto context = ImNodes::Ez::CreateContext();
        IM_UNUSED(context);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        
        if (ImGui::Begin("Billyprints", NULL, window_flags))
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open..", "Ctrl+O")) { }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) { }
                    if (ImGui::MenuItem("Close", "Ctrl+W")) { }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImNodes::Ez::BeginCanvas();

            RenderNodes();

            RenderContextMenu();

            ImNodes::Ez::EndCanvas();
            ImGui::End();
        }
    }

    NodeEditor::NodeEditor() { }
}