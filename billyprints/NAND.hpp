#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class NAND : public Gate
	{
	public:
		NAND();
		bool Evaluate() override;
	};
}
