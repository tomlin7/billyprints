#pragma once

#include "Gate.hpp"

namespace Billyprints {
	class Buffer : public Gate
	{
	public:
		Buffer();
		bool Evaluate() override;
	};
}