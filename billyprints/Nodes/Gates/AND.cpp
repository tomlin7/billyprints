#include "AND.hpp"

namespace Billyprints {
	AND::AND() : Gate("AND", { {"a"}, {"b"} }, { {"out"} }) { }

	bool AND::Evaluate() {
		//bool current = true;
		//foreach(bool b in input) current &= !b;
		//return current;

		int len = 0;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				len++;

		if (len < inputSlotCount) return false;

		bool current = true;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				current &= ((Node*)cn.outputNode)->value;

		value = current;
		return value;
	}
}
