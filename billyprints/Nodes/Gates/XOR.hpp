#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class XOR : public Gate
	{
	public:
		XOR();
		bool Evaluate() override;
	};
}