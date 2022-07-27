#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class NOR : public Gate
	{
	public:
		NOR();
		bool Evaluate() override;
	};
}
