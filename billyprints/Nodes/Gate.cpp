#include "Gate.hpp"

Gate::Gate(const char* _title,
    std::vector<ImNodes::Ez::SlotInfo>&& _inputSlots,
    std::vector<ImNodes::Ez::SlotInfo>&& _outputSlots)
    : Node(_title, std::move(_inputSlots), std::move(_outputSlots))
{ }

bool Gate::Evaluate() { 
    return value;
}
