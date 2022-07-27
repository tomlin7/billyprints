#pragma once

#include "Gate.hpp"

class NOT : public Gate
{
public:
	NOT();
	bool Evaluate() override;
};
