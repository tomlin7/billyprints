#include "Node.hpp"

Node::Node(const char* _title,
    std::vector<ImNodes::Ez::SlotInfo>&& _inputSlots,
    std::vector<ImNodes::Ez::SlotInfo>&& _outputSlots)
{
    title = _title;
    inputSlots = _inputSlots;
    outputSlots = _outputSlots;

    inputSlotCount = static_cast<int>(inputSlots.size());
    outputSlotCount = static_cast<int>(outputSlots.size());
}

void Node::DeleteConnection(const Connection& connection)
{
    for (auto it = connections.begin(); it != connections.end(); ++it)
    {
        if (connection == *it)
        {
            connections.erase(it);
            break;
        }
    }
}

bool Node::Evaluate() {
    return value;
}
