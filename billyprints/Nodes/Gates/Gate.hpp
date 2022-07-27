#pragma once

#include "Node.hpp"

namespace Billyprints {
    class Gate : public Node
    {
    public:
        virtual bool Evaluate();

        Gate(const char* title,
            std::vector<ImNodes::Ez::SlotInfo>&& inputSlots,
            std::vector<ImNodes::Ez::SlotInfo>&& outputSlots);
    };
}