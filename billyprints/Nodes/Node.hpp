#pragma once

#include "pch.hpp"
#include "Connection.hpp"

class Node
{
public:
    /// Node title
    const char* Title = nullptr;
    bool Selected = false;
    ImVec2 Pos{};
    bool Value = false;

    std::vector<Connection> Connections{};
    std::vector<ImNodes::Ez::SlotInfo> InputSlots{};
    std::vector<ImNodes::Ez::SlotInfo> OutputSlots{};

    int InputSlotCount;
    int OutputSlotCount;

    Node(const char* title,
        const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
        const std::vector<ImNodes::Ez::SlotInfo>&& output_slots);
    void DeleteConnection(const Connection& connection);
    virtual bool Evaluate();
};
