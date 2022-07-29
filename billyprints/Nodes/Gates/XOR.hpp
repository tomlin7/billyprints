#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class XOR : public Gate
	{
	public:
		XOR();
		static bool XOR_F(const std::vector<bool>& input, const int&);

		bool Evaluate() override;
	};
}