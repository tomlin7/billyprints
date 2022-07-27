#include "NodeEditor.hpp"

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
        if (ImGui::BeginPopup("NodesContextMenu"))
        {
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
            if (ImGui::MenuItem("Reset Zoom"))
                ImNodes::GetCurrentCanvas()->Zoom = 1;

            if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void NodeEditor::Redraw() {
        auto context = ImNodes::Ez::CreateContext();
        IM_UNUSED(context);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin("ImNodes", NULL, window_flags))
        {
            ImNodes::Ez::BeginCanvas();

            RenderNodes();

            if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
            {
                ImGui::FocusWindow(ImGui::GetCurrentWindow());
                ImGui::OpenPopup("NodesContextMenu");
            }

            RenderContextMenu();

            ImNodes::Ez::EndCanvas();
            ImGui::End();
        }
    }

    NodeEditor::NodeEditor() { }
}