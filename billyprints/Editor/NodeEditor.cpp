#include "NodeEditor.hpp"

inline void NodeEditor::RenderNode(Node* node) {
    if (ImNodes::Ez::BeginNode(node, node->Title, &node->Pos, &node->Selected))
    {
        ImNodes::Ez::InputSlots(node->InputSlots.data(), static_cast<int>(node->InputSlots.size()));
        ImGui::Text("%d", (Node*)node->Evaluate());
        ImNodes::Ez::OutputSlots(node->OutputSlots.data(), static_cast<int>(node->OutputSlots.size()));

        Connection new_connection;
        if (ImNodes::GetNewConnection(&new_connection.InputNode, &new_connection.InputSlot,
            &new_connection.OutputNode, &new_connection.OutputSlot))
        {
            ((Node*)new_connection.InputNode)->Connections.push_back(new_connection);
            ((Node*)new_connection.OutputNode)->Connections.push_back(new_connection);
        }

        // Render connections
        for (const Connection& connection : node->Connections)
        {
            if (connection.OutputNode != node)
                continue;

            if (!ImNodes::Connection(connection.InputNode, connection.InputSlot, connection.OutputNode,
                connection.OutputSlot))
            {
                ((Node*)connection.InputNode)->DeleteConnection(connection);
                ((Node*)connection.OutputNode)->DeleteConnection(connection);
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

        if (node->Selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete) && ImGui::IsWindowFocused())
        {
            for (auto& connection : node->Connections)
            {
                if (connection.OutputNode == node)
                {
                    ((Node*)connection.InputNode)->DeleteConnection(connection);
                }
                else
                {
                    ((Node*)connection.OutputNode)->DeleteConnection(connection);
                }
            }
            node->Connections.clear();

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
        for (const auto& desc : available_nodes)
        {
            auto item = desc();
            if (ImGui::MenuItem(item->Title))
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

NodeEditor::NodeEditor() {
    
}
