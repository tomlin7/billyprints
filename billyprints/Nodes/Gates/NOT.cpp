#include "NOT.hpp"

namespace Billyprints {
	NOT::NOT() : Gate("NOT", { {"in"} }, { {"out"} }) { }

	bool NOT::NOT_F(const std::vector<bool>& input, const int&) {
		for (const bool& pin : input)
			if (!pin) return !pin;
		
		return false;
	}

	bool NOT::Evaluate() {
		std::vector<bool> input;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				input.push_back(((Node*)cn.outputNode)->value);

		value = NOT_F(input, inputSlotCount);
		return value;
	}
}
