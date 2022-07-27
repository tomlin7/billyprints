#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class NOT : public Gate
	{
	public:
		NOT();
		bool Evaluate() override;
	};
}
