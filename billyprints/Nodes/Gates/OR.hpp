#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class OR : public Gate
	{
	public:
		OR();
		static bool OR_F(const std::vector<bool>& input, const int&);

		bool Evaluate() override;
	};
}
