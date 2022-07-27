#include "NAND.hpp"

namespace Billyprints {
	NAND::NAND() : Gate("NAND", { {"a"}, {"b"} }, { {"out"} }) { }

	bool NAND::Evaluate() {
		//bool current = true;
		//foreach(bool b in input) current &= !b;
		//return !current;

		bool current = true;
		for (const auto& cn : connections)
			if (cn.inputNode == this)
				current &= ((Node*)cn.outputNode)->value;

		value = !current;
		return value;
	}
}
