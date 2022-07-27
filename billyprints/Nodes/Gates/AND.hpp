#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class AND : public Gate
	{
	public:
		AND();
		bool Evaluate() override;
	};
}
