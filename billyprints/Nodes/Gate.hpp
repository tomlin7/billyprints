#pragma once

#include "Node.hpp"

class Gate : public Node
{
public:
    virtual bool Evaluate();

    Gate(const char* title,
        const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
        const std::vector<ImNodes::Ez::SlotInfo>&& output_slots);
};
