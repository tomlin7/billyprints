#include "NAND.hpp"

namespace Billyprints {
	NAND::NAND() : Gate("NAND", { {"a"}, {"b"} }, { {"out"} }) { }

	bool NAND::Evaluate() {
		//if (empty) return;
		//foreach(var b in x)
		//	if (!b) return true;

		//return false;

		int len = 0;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				len++;

		if (len < inputSlotCount) return false;

		// ALL
		for (const auto& cn : connections) {
			if (cn.inputNode == this && !((Node*)cn.outputNode)->value) {
				value = true;
				return value;
			}
		}

		value = false;
		return value;
	}
}
