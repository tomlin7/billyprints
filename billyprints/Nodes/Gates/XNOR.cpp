#include "XNOR.hpp"

namespace Billyprints {
	XNOR::XNOR() : Gate("XNOR", { {"a"}, {"b"} }, { {"out"} }) { }

	bool XNOR::Evaluate() {
		//bool current = false;
		//foreach(bool b in input) current ^= b;
		//return !current;

		bool current = false;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				current ^= ((Node*)cn.outputNode)->value;

		value = !current;
		return value;
	}
}