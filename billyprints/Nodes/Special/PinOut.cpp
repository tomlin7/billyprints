#include "PinOut.hpp"

namespace Billyprints {
	PinOut::PinOut() : Node("Pin Out", { {"in"} }, {}) {
		value = true;
	};

	bool PinOut::Evaluate() {
		for (const auto& cn : connections) {
			if (cn.inputNode == this && ((Node*)cn.outputNode)->value) {
				value = true;
				return value;
			}
		}

		value = false;
		return value;
	};
}
