#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class NOT : public Gate
	{
	public:
		NOT();
		static bool NOT_F(const std::vector<bool>& input, const int&);

		bool Evaluate() override;
	};
}
