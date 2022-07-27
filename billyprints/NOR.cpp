#include "NOR.hpp"

namespace Billyprints {
	NOR::NOR() : Gate("NOR", { {"a"}, {"b"} }, { {"out"} }) { }

	bool NOR::Evaluate() {
		//foreach(var b in x)
		//	if (b) return true;

		//return false;

		// ANY
		for (const auto& cn : connections) {
			if (cn.inputNode == this && ((Node*)cn.outputNode)->value) {
				value = false;
				return value;
			}
		}

		value = true;
		return value;
	}
}
