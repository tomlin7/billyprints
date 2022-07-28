#include "Buffer.hpp"

namespace Billyprints {
	Buffer::Buffer() : Gate("Buffer", { {"in"} }, { {"out"} }) { }

	bool Buffer::Evaluate() {
		for (const auto& cn : connections){
			if (cn.inputNode == this){
				value = ((Node*)cn.outputNode)->value;
				return value;
			}
		}

		value = false;
		return value;
	}
}