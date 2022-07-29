#pragma once

#include "Gate.hpp"
#include "XOR.hpp"

namespace Billyprints {
	class XNOR : public Gate
	{
	public:
		XNOR();
		static bool XNOR_F(const std::vector<bool>& input, const int& pinCount);

		bool Evaluate() override;
	};
}