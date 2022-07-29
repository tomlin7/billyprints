#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class AND : public Gate
	{
	public:
		AND();
		static bool AND_F(const std::vector<bool>& input, const int& pinCount);

		bool Evaluate() override;
	};
}
