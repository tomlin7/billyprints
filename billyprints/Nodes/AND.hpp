#pragma once

#include "Gate.hpp"

class AND : public Gate
{
public:
	AND();
	bool Evaluate() override;

};

