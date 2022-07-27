#include "Source.hpp"

namespace Billyprints {
	Source::Source() : Node("Source", {}, { {"out"} }) {
		value = true;
	};

	bool Source::Evaluate() {
		return value;
	};
}
