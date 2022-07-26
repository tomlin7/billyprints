#pragma once

#include "Gate.hpp"

class OR : public Gate
{
public:
	OR();
	bool Evaluate() override;
};

