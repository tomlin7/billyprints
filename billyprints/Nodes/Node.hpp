#pragma once

#include "pch.hpp"
#include "Connection.hpp"

class Node
{
public:
    /// Node title
    const char* title = nullptr;
    bool selected = false;
    ImVec2 pos{};
    bool value = false;

    std::vector<Connection> connections{};
    std::vector<ImNodes::Ez::SlotInfo> inputSlots{};
    std::vector<ImNodes::Ez::SlotInfo> outputSlots{};

    int inputSlotCount;
    int outputSlotCount;

    Node(const char* title,
        std::vector<ImNodes::Ez::SlotInfo>&& _inputSlots,
        std::vector<ImNodes::Ez::SlotInfo>&& _outputSlots);
    void DeleteConnection(const Connection& connection);
    virtual bool Evaluate();
};
