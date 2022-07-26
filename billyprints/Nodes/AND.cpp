#include "AND.hpp"

AND::AND() : Gate("AND", { {"a"}, {"b"} }, { {"out"} }) { }

bool AND::Evaluate() {
	//if (empty) return;
	//foreach(var b in x)
	//	if (!b) return false;

	//return true;

	int len = 0;
	for (const auto& cn : Connections)
		if (cn.InputNode == this)
			len++;

	if (len < InputSlotCount) return false;

	// ALL
	for (const auto& cn : Connections) {
		if (cn.InputNode == this && !((Node*)cn.OutputNode)->Value) {
			Value = false;
			return Value;
		}
	}

	Value = true;
	return Value;
}
