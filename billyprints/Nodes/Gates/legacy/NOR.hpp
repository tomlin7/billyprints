#pragma once

#include "Gate.hpp"
#include "OR.hpp"

namespace Billyprints {
	class NOR : public Gate
	{
	public:
		NOR();
		static bool NOR_F(const std::vector<bool>& input, const int& pinCount);

		bool Evaluate() override;
	};
}
