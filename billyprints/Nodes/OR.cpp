#include "OR.hpp"

OR::OR() : Gate("OR", { {"a"}, {"b"} }, { {"out"} }) { }

bool OR::Evaluate() {
	//foreach(var b in x)
	//	if (b) return true;

	//return false;
	
	// ANY
	for (const auto &cn : Connections) {
		if (cn.InputNode == this && ((Node*)cn.OutputNode)->Value) {
			Value = true;
			return Value;
		}
	}
	
	Value = false;
	return Value;
}
