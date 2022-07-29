#include "PinIn.hpp"

namespace Billyprints {
	PinIn::PinIn() : Node("Pin In", {}, { {"out"} }) {
		value = true;
	};

	bool PinIn::Evaluate() {
		return value;
	};
}
