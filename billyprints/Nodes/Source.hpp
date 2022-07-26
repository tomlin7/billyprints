#pragma once

#include "Node.hpp"

class Source : public Node
{
public:
	Source();
	bool Evaluate() override;
};
