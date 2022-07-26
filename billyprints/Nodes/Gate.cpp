#include "Gate.hpp"

Gate::Gate(const char* title,
    const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
    const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
    : Node(title, std::move(input_slots), std::move(output_slots))
{ }

bool Gate::Evaluate() { 
    return Value;
}
