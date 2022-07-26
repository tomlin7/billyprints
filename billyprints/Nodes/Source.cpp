#include "Source.hpp"

Source::Source() : Node("Source", {}, { {"out"} }) {
	Value = true;
};

bool Source::Evaluate() {
	return Value;
};
