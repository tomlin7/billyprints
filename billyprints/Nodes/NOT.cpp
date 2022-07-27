#include "NOT.hpp"

NOT::NOT() : Gate("NOT", { {"in"} }, { {"out"} }) { } 

bool NOT::Evaluate() {
	for (const auto& cn : connections) {
		if (cn.inputNode == this) {
			value = !((Node*)cn.outputNode)->value;
			return false;
		}
	}

	value = false;
	return value;
}
