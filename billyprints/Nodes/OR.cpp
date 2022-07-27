#include "OR.hpp"

namespace Billyprints {
	OR::OR() : Gate("OR", { {"a"}, {"b"} }, { {"out"} }) { }

	bool OR::Evaluate() {
		//foreach(var b in x)
		//	if (b) return true;

		//return false;

		// ANY
		for (const auto& cn : connections) {
			if (cn.inputNode == this && ((Node*)cn.outputNode)->value) {
				value = true;
				return value;
			}
		}

		value = false;
		return value;
	}
}
