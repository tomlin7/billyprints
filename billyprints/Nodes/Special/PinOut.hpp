#pragma once

#include "Node.hpp"

namespace Billyprints {
	class PinOut : public Node
	{
	public:
		PinOut();
		bool Evaluate() override;
	};
}
