#include "Source.hpp"

Source::Source() : Node("Source", {}, { {"out"} }) {
	value = true;
};

bool Source::Evaluate() {
	return value;
};
