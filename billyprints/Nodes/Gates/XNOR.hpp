#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class XNOR : public Gate
	{
	public:
		XNOR();
		bool Evaluate() override;
	};
}