#pragma once

#include "Node.hpp"

namespace Billyprints {
	class Source : public Node
	{
	public:
		Source();
		bool Evaluate() override;
	};
}
