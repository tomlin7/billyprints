#include "Node.hpp"

Node::Node(const char* title,
    const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
    const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
{
    Title = title;
    InputSlots = input_slots;
    OutputSlots = output_slots;

    InputSlotCount = static_cast<int>(input_slots.size());
    OutputSlotCount = static_cast<int>(output_slots.size());
}

void Node::DeleteConnection(const Connection& connection)
{
    for (auto it = Connections.begin(); it != Connections.end(); ++it)
    {
        if (connection == *it)
        {
            Connections.erase(it);
            break;
        }
    }
}

bool Node::Evaluate() {
    return Value;
}
