#pragma once

#include "pch.hpp"
#include "Node.hpp"

namespace Billyprints {
    class Gate : public Node
    {
    public:
        Gate(const char* title,
            std::vector<ImNodes::Ez::SlotInfo>&& inputSlots,
            std::vector<ImNodes::Ez::SlotInfo>&& outputSlots);

        virtual void Render() override;
        virtual ImU32 GetColor() const override;
    };
}
