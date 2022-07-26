#include "AND.hpp"

AND::AND() : Gate("AND", { {"a"}, {"b"} }, { {"out"} }) { }

bool AND::Evaluate() {
	//if (empty) return;
	//foreach(var b in x)
	//	if (!b) return false;

	//return true;

	int len = 0;
	for (const auto& cn : connections)
		if (cn.inputNode == this)
			len++;

	if (len < inputSlotCount) return false;

	// ALL
	for (const auto& cn : connections) {
		if (cn.inputNode == this && !((Node*)cn.outputNode)->value) {
			value = false;
			return value;
		}
	}

	value = true;
	return value;
}
