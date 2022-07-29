#pragma once

#include "Node.hpp"

namespace Billyprints {
	class PinIn : public Node
	{
	public:
		PinIn();
		bool Evaluate() override;
	};
}
