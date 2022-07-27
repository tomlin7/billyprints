#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class OR : public Gate
	{
	public:
		OR();
		bool Evaluate() override;
	};
}
